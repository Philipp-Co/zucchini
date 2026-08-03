#ifndef __ZUCCHINI_IPC_KEY_H__
#define __ZUCCHINI_IPC_KEY_H__

#include <sys/ipc.h>

key_t ZUC_IpcKeyFromName(const char *name);

#endif
