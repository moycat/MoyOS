/*
 * port.h @ MoyOS # STM32F10x
 *
 * This file should implement device-related structs,
 * as well as types like uint8_t, uint16_t.
 *
 */
#ifndef MOYOS_PORT_H
#define MOYOS_PORT_H

#include "../CMSIS/CM3/DeviceSupport/ST/STM32F10x/stm32f10x.h"
#include "../CMSIS/CM3/CoreSupport/core_cm3.h"

/* size_t should be defined as "moy_size" */
typedef uint32_t moy_size;

/* For using stored registers in stack. */
typedef struct {
    /* Saved by program manually */
    uint32_t r4;
    uint32_t r5;
    uint32_t r6;
    uint32_t r7;
    uint32_t r8;
    uint32_t r9;
    uint32_t r10;
    uint32_t r11;
} MoyFrame;

typedef struct {
    /* Saved by hardware automatically */
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
} AutoFrame;

#endif //MOYOS_PORT_H
