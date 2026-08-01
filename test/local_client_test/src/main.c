#include <zucchini/client/client.h>

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define ZUC_TEST_RING_BUFFER_NAME "zucchini"
#define ZUC_TEST_BUFFER_SIZE 1024

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    ZUC_Client_t client = ZUC_CreateClient(ZUC_TEST_RING_BUFFER_NAME);

    uint8_t buffer[ZUC_TEST_BUFFER_SIZE];
    uint16_t size;
    for(;;)
    {
        while(0 == ZUC_ClientReceive(client, buffer, &size))
        {
            printf("Received %u bytes: %.*s\n", size, size, (char*)buffer);
            ZUC_ClientSend(client, buffer, size);
        }
        ZUC_ClientWait(client, 1000);
    }

    ZUC_DestroyClient(&client);
    return 0;
}
