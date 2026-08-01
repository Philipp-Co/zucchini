#ifndef __ZUCCHINI_CLIENT_CLIENT_H__
#define __ZUCCHINI_CLIENT_CLIENT_H__

#include <stdint.h>

struct ZUC_Client;
typedef struct ZUC_Client* ZUC_Client_t;

ZUC_Client_t ZUC_CreateClient(const char *name);
void ZUC_DestroyClient(ZUC_Client_t *client);

int32_t ZUC_ClientWait(ZUC_Client_t client, int32_t timeout_ms);
int32_t ZUC_ClientReceive(ZUC_Client_t client, void *out_data, uint16_t *out_size);
int32_t ZUC_ClientSend(ZUC_Client_t client, const void *data, uint16_t size);

#endif
