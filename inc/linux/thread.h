#ifndef __INC_LINUX_THREAD__
#define __INC_LINUX_THREAD__

#include <semaphore.h>
#include <pthread.h>

typedef sem_t* sem_t_id;
typedef pthread_mutex_t mux_t;
typedef pthread_mutex_t* mux_t_id;

sem_t_id semCreate(int _value);
int semTake(sem_t_id _sem, int _ms);
int semGive(sem_t_id _sem);

mux_t_id muxCreate();
int muxLock(mux_t_id _mux);
int muxUnlock(mux_t_id _mux);

int taskCreate(void * (*fun)(void *),void *arg);
#endif
