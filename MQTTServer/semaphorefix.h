#ifndef SEMAPHOREFIX_H
#define SEMAPHOREFIX_H

#ifdef __APPLE__
#include <dispatch/dispatch.h>
#else
#include <semaphore.h>
#endif

struct SemaFix {
#ifdef __APPLE__
    dispatch_semaphore_t    sem;
#else
    sem_t                   sem;
#endif

    SemaFix() = default;

    SemaFix(uint32_t value)
    {
#ifdef __APPLE__
    sem = dispatch_semaphore_create(value);
#else
    sem_init(&sem, 0, value);
#endif
    }

    inline void Init(uint32_t value)
    {
    #ifdef __APPLE__
        sem = dispatch_semaphore_create(value);
    #else
        sem_init(&sem, 0, value);
    #endif
    }

    inline void Wait()
    {

    #ifdef __APPLE__
        dispatch_semaphore_wait(sem, DISPATCH_TIME_FOREVER);
    #else
        int r;

        do {
                r = sem_wait(&sem);
        } while (r == -1 && errno == EINTR);
    #endif
    }

    inline void Post()
    {
    #ifdef __APPLE__
        dispatch_semaphore_signal(sem);
    #else
        sem_post(&sem);
    #endif
    }
};



#endif // SEMAPHOREFIX_H
