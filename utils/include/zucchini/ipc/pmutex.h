#ifndef __ZUCCHINI_IPC_PMUTEX_H__
#define __ZUCCHINI_IPC_PMUTEX_H__

#include <stdint.h>

struct ZUC_PMutex;
typedef struct ZUC_PMutex* ZUC_PMutex_t;

ZUC_PMutex_t ZUC_CreatePMutex(const char *name);
void ZUC_DestroyPMutex(ZUC_PMutex_t *mutex);

void ZUC_PMutexLock(ZUC_PMutex_t mutex);
int32_t ZUC_PMutexTryLock(ZUC_PMutex_t mutex);
void ZUC_PMutexUnlock(ZUC_PMutex_t mutex);

#endif
