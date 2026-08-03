#include "key.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

key_t ZUC_IpcKeyFromName(const char *name)
{
    char path[256];
    snprintf(path, sizeof(path), "/tmp/.zucchini_ipc_%s", name);

    const int fd = open(path, O_CREAT | O_RDONLY, 0666);
    if(fd != -1)
    {
        close(fd);
    }
    return ftok(path, 'Z');
}
