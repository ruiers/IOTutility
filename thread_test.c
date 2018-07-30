#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "inc/base.h"

 
sem_t_id ready;

void* thread()
{
    taskSetName("THREAD1");
    while (1)
    {
		
        semTake(ready, -1);
        printf("print from %s\n", __func__);
    }
}
void main()
{
    taskSetName("MainThread");
    ready = semCreate(0);

    printf("test start\n");

    taskCreate(thread, NULL);

    while (1)
    {
        semGive(ready);
        printf("print from %s\n", __func__);
        sleep(5);
    }
    return;
}
