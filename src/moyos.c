/*
 * moyos.c @ MoyOS
 *
 * Main body of OS.
 *
 */
#include "moyos.h"
#include "helper.h"

/* Task control blocks. */
MoyTCB tasks[MOY_TASK_SIZE];
uint8_t task_count = 0;
uint8_t current_task = (uint8_t)-1;
uint8_t idle_task_id;

/* Queues. */
MoyQueue queues[MOY_QUEUE_SIZE];
uint8_t queue_count = 0;

/* Status. */
uint8_t started = 0;
uint8_t critical_depth = 0;

/* Memory pool to be allocated from. */
moy_size *pool[MOY_POOL_SIZE];
moy_size pool_count = 0;

/*
 * Allocate a stack from pool.
 */
moy_size* _moyAllocStack(uint32_t stack_size)
{
    moyEnterCritical();
    /* Check if pool is full. */
    if (pool_count + stack_size >= MOY_POOL_SIZE) {
        return 0;
    }
    pool_count += stack_size;
    moyLeaveCritical();
    return (moy_size*)(pool + pool_count);
}

/*
 * Start everything.
 */
void moyStart()
{
    /* Start the idle task. */
    uint8_t result;
    result = moyCreateTask((TaskFunction) _moyIdleTask, "idle", 24, 0, 0, &idle_task_id);
    if (result != TASK_OK) {
        return;
    }

    /* Start the OS */
    _moyInitTicker();
    _moySyscall(SYSCALL_START_OS, 0, 0, 0);
}

/*
 * Check OS Status.
 */
uint8_t moyIsRunning()
{
    return started;
}

/*
 * Sleep a time.
 */
void moyDelay(moy_size sleep_time)
{
    _moySyscall(SYSCALL_TASK_SLEEP, sleep_time, current_task, 0);
    _moyYield();
}

/*
 * Create a task, and get a handler to operate it.
 * Return a status code.
 * Priority should be over 0 except the idle task.
 */
uint8_t moyCreateTask(
        TaskFunction entry,
        const char *name,
        moy_size stack_size,
        void *parameters,
        uint8_t priority,
        uint8_t *handler
)
{
    moyEnterCritical();
    /* Check if reaching maximum task number */
    if (task_count == MOY_TASK_SIZE) {
        moyLeaveCritical();
        return TASK_MAXIMUM_EXCEEDED;
    }
    /* Check if pool is full */
    void *stack_bottom = _moyAllocStack(stack_size);
    if (stack_bottom == 0) {
        moyLeaveCritical();
        return TASK_MEM_POOL_FULL;
    }
    /* Set handler */
    if (handler != 0) {
        *handler = task_count;
    }
    /* Init TCB */
    MoyTCB *this_task = tasks + task_count++;
    this_task->stack_size = stack_size;
    this_task->priority = priority;
    this_task->status = TASK_READY;
    this_task->stack_bottom = (moy_size)stack_bottom;
    _moyInitFrame(this_task, entry, parameters);
    if (name != 0) {
        strcpy(this_task->name, name);
    }
    moyLeaveCritical();
    return TASK_OK;
}

/*
 * Del current task.
 */
void moyDelTask()
{
    if (!started) return;
    moyDelTaskByID(current_task);
    _moyYield();
}

/*
 * Del a task by ID.
 */
void moyDelTaskByID(uint8_t handler)
{
    tasks[handler].status = 0;
}

/*
 * Find the next task to execute.
 */
static inline MoyTCB* FindAvaTask()
{
    moyEnterCritical();
    uint8_t this_task_id = current_task;

    /* Idle task as default. */
    uint8_t next_task_id = idle_task_id;
    uint8_t highest_priority = 0;
    uint8_t i = task_count;

    while (i--) {
        this_task_id++;
        if (this_task_id >= task_count) this_task_id = 0;

        MoyTCB *this_task = tasks + this_task_id;

        /* Check queues if awaiting. */
        if (this_task->status | TASK_BLOCKED_READING_QUEUE) {
            if (queues[this_task->waiting].status == QUEUE_FILLED) {
                this_task->status = TASK_READY;
            }
        }
        if (this_task->status | TASK_BLOCKED_WRITING_QUEUE) {
            if (queues[this_task->waiting].status == QUEUE_EMPTY) {
                this_task->status = TASK_READY;
            }
        }

        /* Check if executable */
        if (this_task->status == TASK_READY
                && this_task->priority > highest_priority) {
            next_task_id = this_task_id;
            highest_priority = this_task->priority;
        }
    }

    current_task = next_task_id;
    moyLeaveCritical();
    return tasks + next_task_id;
}

/*
 * Should be called every tick.
 * Deal with sleep and block.
 */
void _moyTick()
{
    moyEnterCritical();
    int i;
    for (i = 0; i < task_count; ++i) {
        MoyTCB *this_task = tasks + i;
        /* Deal with waiting */
        if (this_task->status &
                (TASK_DELAYED | TASK_BLOCKED_READING_QUEUE | TASK_BLOCKED_WRITING_QUEUE)) {
            if (this_task->sleep_time <= MOY_SWITCH_INTERVAL) {
                this_task->status = TASK_READY;
            } else {
                this_task->sleep_time -= MOY_SWITCH_INTERVAL;
            }
        }
    }
    moyLeaveCritical();
}

/*
 * Should be called when switch occurs.
 * Save the stack top and return the next stack top.
 */
moy_size _moySwitch(moy_size stack_top)
{
    /* Update the stack top */
    tasks[current_task].stack_top = stack_top;

    MoyTCB *next_task = FindAvaTask();

    return next_task->stack_top;
}

/*
 * Create a queue.
 */
uint8_t moyCreateQueue(uint8_t *handle)
{
    moyEnterCritical();
    if (queue_count == MOY_QUEUE_SIZE) {
        moyLeaveCritical();
        return QUEUE_MAXIMUM_EXCEEDED;
    }
    queues[queue_count].status = QUEUE_EMPTY;
    *handle = queue_count++;
    moyLeaveCritical();
    return QUEUE_OK;
}

/*
 * Push an item into a queue.
 * Set timeout to 0 for no waiting.
 */
uint8_t moyQueuePush(uint8_t queue_id, moy_size item, moy_size timeout)
{
    MoyQueue *this_queue = queues + queue_id;
    moyEnterCritical();

    /* If empty, just write and return. */
    if (this_queue->status == QUEUE_EMPTY) {
        this_queue->item_ptr = item;
        this_queue->status = QUEUE_FILLED;
        moyLeaveCritical();
        return QUEUE_OK;
    }

    /* Not empty, wait or fail. */
    if (!timeout) {
        moyLeaveCritical();
        return QUEUE_FAILED;
    }
    MoyTCB *this_task = tasks + current_task;
    this_task->status = TASK_BLOCKED_WRITING_QUEUE;
    this_task->waiting = queue_id;
    this_task->sleep_time = timeout;
    moyLeaveCritical();
    _moyYield();

    /* Now the block has been removed. Let's check the queue. */
    moyEnterCritical();
    if (this_queue->status == QUEUE_EMPTY) {
        this_queue->item_ptr = item;
        this_queue->status = QUEUE_FILLED;
        moyLeaveCritical();
        return QUEUE_OK;
    }
    moyLeaveCritical();
    return QUEUE_FAILED;
}

/*
 * Pull an item from a queue.
 * Set timeout to 0 for no waiting.
 */
uint8_t moyQueuePull(uint8_t queue_id, moy_size *item_ptr, moy_size timeout)
{
    MoyQueue *this_queue = queues + queue_id;
    moyEnterCritical();

    /* If empty, just write and return. */
    if (this_queue->status == QUEUE_FILLED) {
        *item_ptr = this_queue->item_ptr;
        this_queue->status = QUEUE_EMPTY;
        moyLeaveCritical();
        return QUEUE_OK;
    }

    /* Not empty, wait or fail. */
    if (!timeout) {
        moyLeaveCritical();
        return QUEUE_FAILED;
    }
    MoyTCB *this_task = tasks + current_task;
    this_task->status = TASK_BLOCKED_READING_QUEUE;
    this_task->waiting = queue_id;
    this_task->sleep_time = timeout;
    moyLeaveCritical();
    _moyYield();

    /* Now the block has been removed. Let's check the queue. */
    moyEnterCritical();
    if (this_queue->status == QUEUE_FILLED) {
        *item_ptr = this_queue->item_ptr;
        this_queue->status = QUEUE_EMPTY;
        moyLeaveCritical();
        return QUEUE_OK;
    }
    moyLeaveCritical();
    return QUEUE_FAILED;
}

/*
 * Make this task sleep.
 */
static inline moy_size _moySvcDoTaskSleep(moy_size sleep_time)
{
    moyEnterCritical();
    tasks[current_task].sleep_time = sleep_time;
    tasks[current_task].status |= TASK_DELAYED;
    moyLeaveCritical();
    return SYSCALL_OK;
}

/*
 * Start the OS.
 */
static inline moy_size _moySvcDoStartOS()
{
    /* Find the available task with the highest priority. */
    MoyTCB *task = FindAvaTask();

    /* Load the context. */
    started = 1;
    _moyLoadContext(task);

    /* Shouldn't be here */
    started = 0;
    return SYSCALL_FAILED;
}

/*
 * Handle syscalls.
 */
moy_size _moySvcHandler(moy_size arg1, moy_size arg2, moy_size arg3, moy_size arg4)
{
    switch (arg1) {
        case SYSCALL_START_OS:
            return _moySvcDoStartOS();
        case SYSCALL_TASK_SLEEP:
            return _moySvcDoTaskSleep(arg2);
        default:
            return SYSCALL_UNDEFINED;
    }
}

/*
 * Enter critical area.
 */
void moyEnterCritical()
{
    critical_depth++;
    __set_PRIMASK(1);
}

/*
 * Leave critical area.
 */
void moyLeaveCritical()
{
    if (critical_depth == 0) return;

    critical_depth--;
    if (critical_depth == 0) {
        __set_PRIMASK(0);
    }
}
