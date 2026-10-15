#ifndef __ZUCCHINI_API_API_H__
#define __ZUCCHINI_API_API_H__

#include <stdint.h>

#include <zucchini/ipc/message.h>

struct ZUC_Api;
typedef struct ZUC_Api* ZUC_Api_t;

ZUC_Api_t ZUC_CreateApi(const char *name);
void ZUC_DestroyApi(ZUC_Api_t *api);

int32_t ZUC_ApiWait(ZUC_Api_t api, int32_t timeout_ms);
int32_t ZUC_ApiReceive(ZUC_Api_t api, uint64_t *out_source, void *out_data, uint16_t *out_size);
int32_t ZUC_ApiSendTo(ZUC_Api_t api, uint64_t code, const void *data, uint16_t size);
int32_t ZUC_ApiBroadcast(ZUC_Api_t api, const void *data, uint16_t size);

#endif
