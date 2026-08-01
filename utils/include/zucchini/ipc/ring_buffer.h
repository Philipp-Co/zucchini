#ifndef __ZUCCHINI_IPC_RING_BUFFER_H__
#define __ZUCCHINI_IPC_RING_BUFFER_H__

#include <stdint.h>

#define ZUC_RING_BUFFER_SLOT_SIZE 1024
#define ZUC_RING_BUFFER_SLOT_COUNT 128

struct ZUC_RingBuffer;
typedef struct ZUC_RingBuffer* ZUC_RingBuffer_t;

ZUC_RingBuffer_t ZUC_CreateRingBuffer(const char *name);
void ZUC_DestroyRingBuffer(ZUC_RingBuffer_t *ring_buffer);

int32_t ZUC_RingBufferPush(ZUC_RingBuffer_t ring_buffer, const void *data, uint16_t size);
int32_t ZUC_RingBufferPop(ZUC_RingBuffer_t ring_buffer, void *out_data, uint16_t *out_size);

#endif
