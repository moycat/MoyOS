/*
 * user_main.c @ MoyOS
 *
 * User program should be written here.
 *
 */
#include "moyos.h"
#include "user_main.h"
#include <stdio.h>

void test1()
{
    int count = 0;
    for (;;) {
        printf("Hello, World! Oak~ %d\n", count++);
        moyDelay(300);
    }
}

void test2()
{
    int count = 0;
    for (;;) {
        printf("Hello, World! Miaow~ %d\n", count++);
        moyDelay(300);
    }
}

void test3()
{
    int count = 0;
    for (;;) {
        printf("Hello, World! Rua~ %d\n", count++);
        moyDelay(500);
    }
}

void test(char* text)
{
    int count = 0;
    for (;;) {
        printf("Hello, World! %s~ %d\n", text, count++);
        moyDelay(1000);
    }
}

char name[] = "moymoy";

void user_main()
{
    setbuf(stdout, NULL);
    moyCreateTask((TaskFunction)test1, 0, 500, 0, 1, 0);
    moyCreateTask((TaskFunction)test2, 0, 500, 0, 1, 0);
    moyCreateTask((TaskFunction)test3, 0, 500, 0, 1, 0);
    moyCreateTask((TaskFunction)test, 0, 800, name, 1, 0);
    moyStart();
}