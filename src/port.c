/*
 * port.c @ MoyOS # STM32F10x
 *
 * This file should implement device-related functions,
 * such as enabling ticker, save & restore registers, etc.
 *
 */
#include "moyos.h"

/*
 * Run when no other tasks can run.
 */
void _moyIdleTask()
{
    /* Ask CPU to sleep, though awaken on interruption */
    while (1) {
        __asm__ ("wfe");
    }
}

/*
 * Force to load the context of some task.
 * Called only in interruption.
 */
void _moyLoadContext(MoyTCB* task)
{
    register uint32_t stack_top asm("r0") = task->stack_top;
    __asm__ __volatile__ (
        R"(
        ldmia %0!, {r4-r11}
        msr psp, %0
        ldr r0, =0xFFFFFFFD
        bx r0
        )"
        :
        : "r" (stack_top)
    );
}

/*
 * Get parameters in stack and call the real handler of OS.
 */
static void ArgPasser(AutoFrame *frame)
{
    /* Check if syscall number is legal. */
    if (*(uint8_t *)(frame->pc - 2) != 0x80) {
        return;
    }

    moy_size result = _moySvcHandler(frame->r0, frame->r1, frame->r2, frame->r3);
    /* Modify r0 in stack directly. */
    frame->r0 = result;
}

/*
 * Defined by CMSIS, Syscall handler.
 * Pass parameters to the real handler of OS.
 * Pure assembly so that the previous stack remains unchanged.
 */
__attribute__((naked)) void SVC_Handler(void)
{
    /* Read SVC arguments from stack. */
    __asm__(
        R"(
        tst lr, #4
        ite eq
        mrseq r0, msp
        mrsne r0, psp
        push {lr}
        bl ArgPasser
        pop {pc}
        )"
    );
}

/*
 * Syscall wrapper.
 */
moy_size _moySyscall(moy_size arg1, moy_size arg2, moy_size arg3, moy_size arg4)
{
    moy_size result;
    __asm__ __volatile__ (
        R"(
        mov r0, %1
        mov r1, %2
        mov r2, %3
        mov r3, %4
        svc #0x80
        mov %0, r0
        )"
        : "=r" (result)
        : "r" (arg1), "r" (arg2), "r" (arg3), "r" (arg4)
        : "r0", "r1", "r2", "r3"
    );
    return result;
}

/*
 * Initialize the ticker, required by OS.
 */
void _moyInitTicker()
{
    /* Set tick frequency to switch frequency and enable it. */
    SysTick_Config(SystemCoreClock / 1000 * MOY_SWITCH_INTERVAL);

    NVIC_SetPriorityGrouping(0);
    NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(0, 0, 0));
    NVIC_SetPriority(SVCall_IRQn, NVIC_EncodePriority(0, 1, 0));
    NVIC_SetPriority(PendSV_IRQn, NVIC_EncodePriority(0, 2, 0));
}

/*
 * Request a instant context switch.
 * Set PendSV in this port.
 */
void _moyYield()
{
    SCB->ICSR = SCB_ICSR_PENDSVSET;
}

/*
 * Defined by CMSIS, called every tick.
 * Set PendSV active. This will call PendSV_Handler.
 */
void SysTick_Handler(void)
{
    if (!moyIsRunning()) return;
    _moyTick();
    _moyYield();
}

/*
 * Defined by CMSIS, called on PendSV.
 * Save context and call OS handler.
 */
__attribute__((naked)) void PendSV_Handler()
{
    __asm__ __volatile__ (
        R"(
        push {lr}
        mrs r0, psp
        stmdb r0!, {r4-r11}
        bl _moySwitch
        )"
        /* moyDoTick should return the stack top of the next task. */
        R"(
        ldmia r0!, {r4-r11}
        msr psp, r0
        pop {pc}
        )"
    );
}

/*
 * Initialize the MCU.
 */
void _moyInit()
{
    /* Initialize the hardware. */
    SystemInit();
}

/*
 * Initialize the frame in TCB.
 */
void _moyInitFrame(MoyTCB *task, TaskFunction entry, void *arg)
{
    /* Registers in AutoFrame will be read automatically by CPU */
    AutoFrame* frame = (AutoFrame*)(task->stack_bottom - sizeof(AutoFrame));
    frame->r0 = (uint32_t)arg;
    frame->pc = (uint32_t)entry;
    frame->lr = (uint32_t)moyDelTask;
    frame->psr = 0x21000000;
    task->stack_top = task->stack_bottom - sizeof(uint32_t) * 16;
}