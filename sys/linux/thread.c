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

sem_t_id semCreate(int _value)
{
    sem_t_id _sem_t;
    _sem_t = (sem_t_id)malloc(sizeof(sem_t));
    if (sem_init(_sem_t, 0, _value) < 0)
    {
        free((void *)_sem_t);
        return NULL;
    }
    return _sem_t;
}

int semTake(sem_t_id _sem, int _ms)
{

    struct timespec timeout;

    if (-1 == _ms)
    {
        if (sem_wait(_sem) < 0) return -1;
        else return 0;
    }

    timeout.tv_sec  = time(NULL) + _ms / 1000;
    timeout.tv_nsec = 0;

    if (_ms % 1000) timeout.tv_sec++;
    if (sem_timedwait(_sem, &timeout) < 0) return -1;
    else return 0;
}
int semGive(sem_t_id _sem)
{
    if (sem_post(_sem) < 0) return -1;
    else return 0;
}

mux_t_id muxCreate()
{
    mux_t_id _mux_t;
    _mux_t = (mux_t_id)malloc(sizeof(mux_t));
    if (pthread_mutex_init(_mux_t, NULL) == -1)
    {
        free((void *)_mux_t);
        return NULL;
    }
    return _mux_t;
}

int muxLock(mux_t_id _mux)
{
    if (pthread_mutex_lock(_mux) < 0) return -1;
    else return 0;
}

int muxUnlock(mux_t_id _mux)
{
    if (pthread_mutex_unlock(_mux) < 0) return -1;
    else return 0;

}
