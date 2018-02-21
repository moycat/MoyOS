/*
 * moyos.h @ MoyOS
 *
 * API list and struct definitions of OS.
 *
 */
#ifndef MOYOS_H
#define MOYOS_H

#include "config.h"

typedef void(*TaskFunction)(void *);

#include "port.h"


/* Task Status Masks */
#define TASK_READY 1
#define TASK_DELAYED (1 << 1)
#define TASK_BLOCKED (1 << 2)


/* Code Definitions */

enum RETURN_CODE {
    SYSCALL_UNDEFINED,
    SYSCALL_OK,
    SYSCALL_FAILED,
    TASK_OK,
    TASK_MAXIMUM_EXCEEDED,
    TASK_MEM_POOL_FULL
};

enum CALL_CODE {
    SYSCALL_START_OS,
    SYSCALL_TASK_SLEEP,
    SYSCALL_SWITCH_CONTEXT,
    SYSCALL_FATAL_ERROR
};


/* OS Structs */

typedef struct {
    char name[MOY_TASK_NAME_SIZE];  /* task name for debug */
    uint8_t status;                 /* task status */
    uint8_t priority;               /* task priority */
    moy_size sleep_time;            /* remaining sleep time */
    moy_size stack_size;            /* size of stack */
    moy_size stack_top;             /* top of task stack */
    moy_size stack_bottom;          /* bottom of task stack */
    MoyFrame frame;                 /* CPU saved status */
} MoyTCB;


/* OS Commands */

void moyStart();

uint8_t moyIsRunning();

void moyEnterCritical();

void moyLeaveCritical();


/* Task Commands */

uint8_t moyCreateTask(
        TaskFunction entry,
        const char *name,
        moy_size stack_size,
        void *parameters,
        uint8_t priority,
        uint8_t *handler
);

void moyDelTask();

void moyDelTaskByID(uint8_t handler);

void moyDelay(moy_size sleep_time);


/* Callees. */

moy_size _moySwitch(moy_size stack_top);

moy_size _moySvcHandler(moy_size arg1, moy_size arg2, moy_size arg3, moy_size arg4);

void _moyYield();

void _moyTick();



/*
 * The following should be implemented in port.c
 */

/* Load the context of some task. */
void _moyLoadContext(MoyTCB* task);

/* Run when no other tasks can run. */
void _moyIdleTask();

/* Called at the very beginning. */
void _moyInit();

/* Do device-related init work. */
void _moyInitFrame(MoyTCB *task, TaskFunction entry, void *arg);

/* Initialize the ticker. */
void _moyInitTicker();

/* Get a stack of some size. */
moy_size* _moyAllocStack(uint32_t stack_size);

/* Syscall Wrapper */
moy_size _moySyscall(moy_size arg1, moy_size arg2, moy_size arg3, moy_size arg4);

#endif //MOYOS_H
