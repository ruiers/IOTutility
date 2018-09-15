#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include "linux/thread.h"

int taskCreate(void* (*fun)(void *), void *arg)
{
    int ret;
    pthread_t thread_self;

    ret = pthread_create(&thread_self, NULL, fun, arg);
    if (ret < 0)
    {
        printf("task start failed: %d", errno);
        return -errno;
    }

    return 0;
}

sem_t_id semCreate(int value_init)
{
    sem_t_id id_sem_t;
    id_sem_t = (sem_t_id)malloc(sizeof(sem_t));
    if (sem_init(id_sem_t, 0, value_init) < 0)
    {
        free((void *)id_sem_t);
        return NULL;
    }
    return id_sem_t;
}

int semTake(sem_t_id id_sem, int ms_time)
{

    struct timespec timeout;

    if (id_sem == NULL)
        return -1;

    if (-1 == ms_time)
    {
        if (sem_wait(id_sem) < 0)
            return -1;
        else
            return 0;
    }

    timeout.tv_sec  = time(NULL) + ms_time / 1000;
    timeout.tv_nsec = 0;

    if (ms_time % 1000)
        timeout.tv_sec++;

    if (sem_timedwait(id_sem, &timeout) < 0)
        return -1;
    else
        return 0;
}
int semGive(sem_t_id id_sem)
{
    if (id_sem == NULL)
        return -1;

    if (sem_post(id_sem) < 0)
        return -1;
    else
        return 0;
}

mux_t_id muxCreate()
{
    mux_t_id id_mux_t;
    id_mux_t = (mux_t_id)malloc(sizeof(mux_t));
    if (pthread_mutex_init(id_mux_t, NULL) == -1)
    {
        free((void *)id_mux_t);
        return NULL;
    }
    return id_mux_t;
}

int muxLock(mux_t_id id_mux)
{
    if (pthread_mutex_lock(id_mux) < 0)
        return -1;
    else
        return 0;
}

int muxTryLock(mux_t_id id_mux)
{
    if (pthread_mutex_trylock(id_mux) == 0)
        return 0;
    else
        return -1;
}

int muxUnlock(mux_t_id id_mux)
{
    if (pthread_mutex_unlock(id_mux) < 0)
        return -1;
    else
        return 0;

}

