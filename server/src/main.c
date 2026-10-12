#include <zucchini/server.h>

#include <errno.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void ZUC_PrintUsage(const char *program);
static void ZUC_SignalHandler(int signum);

int main(int argc, char **argv)
{
    const char *port_argument = NULL;
    const char *name = NULL;
    int option;
    while(-1 != (option = getopt(argc, argv, "p:n:")))
    {
        switch(option)
        {
            case 'p':
                port_argument = optarg;
                break;
            case 'n':
                name = optarg;
                break;
            default:
                ZUC_PrintUsage(argv[0]);
                return 1;
        }
    }

    if(port_argument == NULL || name == NULL || name[0] == '\0' || optind != argc)
    {
        ZUC_PrintUsage(argv[0]);
        return 1;
    }

    char *end = NULL;
    errno = 0;
    const unsigned long port = strtoul(port_argument, &end, 10);
    if(errno != 0 || end == port_argument || *end != '\0' || port == 0 || port > UINT16_MAX)
    {
        fprintf(stderr, "Invalid port '%s' (expected 1-65535)\n", port_argument);
        return 1;
    }

    setvbuf(stdout, NULL, _IOLBF, 0);

    signal(SIGINT, ZUC_SignalHandler);
    signal(SIGTERM, ZUC_SignalHandler);
    signal(SIGPIPE, SIG_IGN);

    if(-1 == ZUC_CreateServer(name, (uint16_t)port))
    {
        return 1;
    }
    ZUC_RunServer();
    ZUC_DestroyServer();

    return 0;
}

static void ZUC_PrintUsage(const char *program)
{
    fprintf(stderr, "Usage: %s -p <port> -n <name>\n", program);
}

static void ZUC_SignalHandler(int signum)
{
    (void)signum;
    ZUC_TerminateServer();
}
