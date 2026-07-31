#include <zucchini/server.h>

#include <signal.h>

static void ZUC_SignalHandler(int signum);

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    signal(SIGINT, ZUC_SignalHandler);

    ZUC_CreateServer();
    ZUC_RunServer();
    ZUC_DestroyServer();

    return 0;
}

static void ZUC_SignalHandler(int signum)
{
    (void)signum;
    ZUC_TerminateServer();
}
