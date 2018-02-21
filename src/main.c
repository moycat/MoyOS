/*
 * main.c @ MoyOS
 *
 * Entry of program, but should be modified.
 * User program should be put in user_main.
 *
 */
#include "moyos.h"
#include "user_main.h"

int main(void)
{
    /* Do init before running. */
    _moyInit();

    /* Do user program. */
    user_main();

    /* Just in case. */
    while(1);
}
