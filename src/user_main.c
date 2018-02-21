/*
 * user_main.c @ MoyOS
 *
 * User program should be written here.
 *
 */
#include "moyos.h"
#include "user_main.h"
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

void test1(uint8_t queue_id)
{
    int count = 0;
    uint32_t item = 233;
    srand(time(NULL));

    for (;;) {
        printf("[Oak] Hello, World! %d\n", count++);
        moyDelay(2000);

        item = (uint8_t)rand();
        uint8_t rs = moyQueuePush(queue_id, item, 0);
        switch (rs) {
            case QUEUE_OK:
                printf("[Oak] Push [%d] OK!\n", item);
                break;
            case QUEUE_FAILED:
                printf("[Oak] Queue not Empty!\n");
                break;
        }
        moyDelay(2000);
    }
}

void test2(uint8_t queue_id)
{
    int count = 0;
    uint32_t item;

    for (;;) {
        printf("[Nut] Hello, World! %d\n", count++);
        moyDelay(3000);

        uint8_t rs = moyQueuePull(queue_id, &item, 0);
        switch (rs) {
            case QUEUE_OK:
                printf("[Nut] Pull [%d] OK!\n", item);
                break;
            case QUEUE_FAILED:
                printf("[Nut] Queue Empty!\n");
                break;
        }
        moyDelay(5000);
    }
}

void user_main()
{
    setbuf(stdout, NULL);

    uint8_t queue;
    moyCreateQueue(&queue);

    moyCreateTask((TaskFunction)test1, 0, 500, queue, 1, 0);
    moyCreateTask((TaskFunction)test2, 0, 500, queue, 1, 0);

    moyStart();
}