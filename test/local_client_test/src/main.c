#include <zucchini/api/api.h>

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define ZUC_TEST_RING_BUFFER_NAME "zucchini"
#define ZUC_TEST_BUFFER_SIZE 1024

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    setvbuf(stdout, NULL, _IOLBF, 0);

    ZUC_Api_t api = ZUC_CreateApi(ZUC_TEST_RING_BUFFER_NAME);

    uint8_t rx_buffer[ZUC_TEST_BUFFER_SIZE];
    uint8_t tx_buffer[ZUC_TEST_BUFFER_SIZE];
    uint16_t size;
    for(;;)
    {
        while(0 == ZUC_ApiReceive(api, rx_buffer, &size))
        {
            printf("Received %u bytes: %.*s\n", size, size, (char*)rx_buffer);
            const int len = snprintf((char*)tx_buffer, sizeof(tx_buffer), "Thanks for: %.*s", size, rx_buffer);
            ZUC_ApiSend(api, tx_buffer, len);
        }
        ZUC_ApiWait(api, 1000);
    }

    ZUC_DestroyApi(&api);
    return 0;
}
