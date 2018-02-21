/*
 * helper.h @ MoyOS
 *
 * Helper functions to get rid of C libraries.
 *
 */
#ifndef MOYOS_HELPER_H
#define MOYOS_HELPER_H

#include "port.h"

void* memcpy(void *destination, const void *source, uint32_t num);
void* memset(void *destination, int value, uint32_t n);
char* strcpy(char *destination, const char *source);

#endif //MOYOS_HELPER_H
