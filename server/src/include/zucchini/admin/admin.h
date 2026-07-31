#ifndef __ZUCCHINI_ADMIN_H__
#define __ZUCCHINI_ADMIN_H__

#include <stdint.h>
#include <netinet/in.h>

#include <zucchini/io/networking/tcp.h>


#define ZUCCHINI_ADMIN_MAX_CLIENTS 4

typedef struct
{
    int a;
} ZUC_UdpWhitelist_t;

typedef struct
{
    ZUC_UdpWhitelist_t upd_whitelist;
} ZUC_Admin_t;


void ZUC_InitializeAdminAdrea(ZUC_Admin_t *area);
void ZUC_OnAdminMessage(ZUC_TcpMessage_t message);


#endif

