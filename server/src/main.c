#include <zucchini/server.h>

#include <signal.h>
#include <stdio.h>

#define ZUC_SERVER_NAME "zucchini"

static void ZUC_SignalHandler(int signum);

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    setvbuf(stdout, NULL, _IOLBF, 0);

    signal(SIGINT, ZUC_SignalHandler);
    signal(SIGTERM, ZUC_SignalHandler);
    signal(SIGPIPE, SIG_IGN);

    ZUC_CreateServer(ZUC_SERVER_NAME);
    ZUC_RunServer();
    ZUC_DestroyServer();

    return 0;
}

static void ZUC_SignalHandler(int signum)
{
    (void)signum;
    ZUC_TerminateServer();
}
