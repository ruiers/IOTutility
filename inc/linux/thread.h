#ifndef __INC_LINUX_THREAD__
#define __INC_LINUX_THREAD__

#include <semaphore.h>
#include <pthread.h>
#include <sys/prctl.h>

typedef sem_t* sem_t_id;
typedef pthread_mutex_t mux_t;
typedef pthread_mutex_t* mux_t_id;

sem_t_id semCreate(int value_init);
int semTake(sem_t_id id_sem, int ms_time);
int semGive(sem_t_id id_sem);

mux_t_id muxCreate();
int muxLock(mux_t_id id_mux);
int muxUnlock(mux_t_id id_mux);

int taskCreate(void * (*fun)(void *),void *arg);
#define taskSetName(name) prctl(PR_SET_NAME,name);

#endif
