/*
 * config.h @ MoyOS
 *
 * Configurations of OS.
 *
 */
#ifndef MOYOS_CONFIG_H
#define MOYOS_CONFIG_H

/* Size of Memory Pool for Stack and Malloc */
#define MOY_POOL_SIZE 2500

/* Maximum Task Size */
#define MOY_TASK_SIZE 10

/* Maximum Length of Task Name */
#define MOY_TASK_NAME_SIZE 10

/* Interval between Switching Tasks (ms) */
#define MOY_SWITCH_INTERVAL 1

#endif //MOYOS_CONFIG_H
