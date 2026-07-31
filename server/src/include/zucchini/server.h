#ifndef __ZUCCHINI_SERVER_H__
#define __ZUCCHINI_SERVER_H__

#include <stdint.h>

int32_t ZUC_CreateServer(void);
void ZUC_DestroyServer(void);
void ZUC_RunServer(void);
void ZUC_TerminateServer(void);

#endif

