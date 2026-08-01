#ifndef __ZUCCHINI_SERVER_CLIENT_H__
#define __ZUCCHINI_SERVER_CLIENT_H__

#include <stdint.h>

struct ZUC_ClientInterface;
typedef struct ZUC_ClientInterface* ZUC_ClientInterface_t;

ZUC_ClientInterface_t ZUC_CreateClientInterface(const char *name);
void ZUC_DestroyClientInterface(ZUC_ClientInterface_t *interface);

int32_t ZUC_ClientInterfaceSend(ZUC_ClientInterface_t interface, const void *data, uint16_t size);
int32_t ZUC_ClientInterfaceReceive(ZUC_ClientInterface_t interface, void *out_data, uint16_t *out_size);

#endif
