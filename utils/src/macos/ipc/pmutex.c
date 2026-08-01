#include <zucchini/ipc/pmutex.h>

#include <semaphore.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct ZUC_PMutex
{
    sem_t *semaphore;
};


ZUC_PMutex_t ZUC_CreatePMutex(const char *name)
{
    ZUC_PMutex_t mutex = malloc(sizeof(struct ZUC_PMutex));
    mutex->semaphore = NULL;

    char semaphore_name[256];
    snprintf(semaphore_name, sizeof(semaphore_name), "/zucchini_pmutex_%s", name);

    sem_t *semaphore = sem_open(semaphore_name, O_CREAT, 0666, 1);
    if(semaphore == SEM_FAILED)
    {
        printf("Unable to create PMutex '%s': %s (%i)\n", name, strerror(errno), errno);
        free(mutex);
        return NULL;
    }

    mutex->semaphore = semaphore;
    return mutex;
}

void ZUC_DestroyPMutex(ZUC_PMutex_t *mutex)
{
    if((*mutex)->semaphore != NULL)
    {
        sem_close((*mutex)->semaphore);
    }
    // Das benannte Semaphore-Objekt bleibt im System bestehen - Abraeumen ist Aufgabe eines anderen Prozesses.
    free(*mutex);
    *mutex = NULL;
}

void ZUC_PMutexLock(ZUC_PMutex_t mutex)
{
    if(mutex->semaphore == NULL)
    {
        return;
    }
    sem_wait(mutex->semaphore);
}

int32_t ZUC_PMutexTryLock(ZUC_PMutex_t mutex)
{
    if(mutex->semaphore == NULL)
    {
        return -1;
    }

    if(sem_trywait(mutex->semaphore) == -1)
    {
        return -1;
    }
    return 0;
}

void ZUC_PMutexUnlock(ZUC_PMutex_t mutex)
{
    if(mutex->semaphore == NULL)
    {
        return;
    }
    sem_post(mutex->semaphore);
}
