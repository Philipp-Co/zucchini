#include <zucchini/admin/admin.h>
#include <stdio.h>


void ZUC_InitializeAdminAdrea(ZUC_Admin_t *area)
{
}

void ZUC_OnAdminMessage(ZUC_TcpMessage_t message)
{
    printf("Size: %u\n", message.size);
    printf("Tcp-Message: %.*s\n", message.size, (char*)message.data);
}
