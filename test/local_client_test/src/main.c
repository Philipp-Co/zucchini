#include <zucchini/api/api.h>

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define ZUC_TEST_BUFFER_SIZE 1024

int main(int argc, char **argv)
{
    const char *name = NULL;
    int option;
    while(-1 != (option = getopt(argc, argv, "n:")))
    {
        switch(option)
        {
            case 'n':
                name = optarg;
                break;
            default:
                printf("Usage: %s -n <name>\n", argv[0]);
                return 1;
        }
    }

    if(name == NULL || name[0] == '\0' || optind != argc)
    {
        printf("Usage: %s -n <name>\n", argv[0]);
        return 1;
    }

    setvbuf(stdout, NULL, _IOLBF, 0);

    ZUC_Api_t api = ZUC_CreateApi(name);

    uint8_t rx_buffer[ZUC_TEST_BUFFER_SIZE];
    uint8_t tx_buffer[ZUC_TEST_BUFFER_SIZE];
    uint16_t size;
    uint64_t source;
    for(;;)
    {
        while(0 == ZUC_ApiReceive(api, &source, rx_buffer, &size))
        {
            printf("Received %u bytes from 0x%llx: %.*s\n", size, (unsigned long long)source, size, (char*)rx_buffer);
            const int len = snprintf((char*)tx_buffer, ZUC_MESSAGE_MAX_PAYLOAD, "Thanks for: %.*s", size, rx_buffer);
            ZUC_ApiSendTo(api, source, tx_buffer, len);
        }
        ZUC_ApiWait(api, 1000);
    }

    ZUC_DestroyApi(&api);
    return 0;
}
