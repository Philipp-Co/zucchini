#include <zucchini/ipc/ring_buffer.h>
#include <zucchini/ipc/shared_memory.h>

#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>

typedef struct
{
    uint16_t size;
    uint8_t data[ZUC_RING_BUFFER_SLOT_SIZE];
} ZUC_RingBufferSlot_t;

typedef struct
{
    _Atomic uint32_t write_index;
    _Atomic uint32_t read_index;
    ZUC_RingBufferSlot_t slots[ZUC_RING_BUFFER_SLOT_COUNT];
} ZUC_RingBufferData_t;

struct ZUC_RingBuffer
{
    ZUC_SharedMemory_t shared_memory;
    ZUC_RingBufferData_t *data;
};


ZUC_RingBuffer_t ZUC_CreateRingBuffer(const char *name)
{
    ZUC_RingBuffer_t ring_buffer = malloc(sizeof(struct ZUC_RingBuffer));
    ring_buffer->shared_memory = ZUC_CreateSharedMemory(name, sizeof(ZUC_RingBufferData_t));
    ring_buffer->data = (ZUC_RingBufferData_t *)ZUC_SharedMemoryAddress(ring_buffer->shared_memory);
    return ring_buffer;
}

void ZUC_DestroyRingBuffer(ZUC_RingBuffer_t *ring_buffer)
{
    ZUC_DestroySharedMemory(&(*ring_buffer)->shared_memory);
    free(*ring_buffer);
    *ring_buffer = NULL;
}

int32_t ZUC_RingBufferPush(ZUC_RingBuffer_t ring_buffer, const void *data, uint16_t size)
{
    if(!ring_buffer->data || size > ZUC_RING_BUFFER_SLOT_SIZE)
    {
        return -1;
    }

    const uint32_t write_index = atomic_load_explicit(&ring_buffer->data->write_index, memory_order_relaxed);
    const uint32_t read_index = atomic_load_explicit(&ring_buffer->data->read_index, memory_order_acquire);

    if(write_index - read_index >= ZUC_RING_BUFFER_SLOT_COUNT)
    {
        return -1;
    }

    ZUC_RingBufferSlot_t *slot = &ring_buffer->data->slots[write_index % ZUC_RING_BUFFER_SLOT_COUNT];
    memcpy(slot->data, data, size);
    slot->size = size;

    atomic_store_explicit(&ring_buffer->data->write_index, write_index + 1, memory_order_release);
    return 0;
}

int32_t ZUC_RingBufferPop(ZUC_RingBuffer_t ring_buffer, void *out_data, uint16_t *out_size)
{
    if(!ring_buffer->data)
    {
        return -1;
    }

    const uint32_t read_index = atomic_load_explicit(&ring_buffer->data->read_index, memory_order_relaxed);
    const uint32_t write_index = atomic_load_explicit(&ring_buffer->data->write_index, memory_order_acquire);

    if(read_index == write_index)
    {
        return -1;
    }

    ZUC_RingBufferSlot_t *slot = &ring_buffer->data->slots[read_index % ZUC_RING_BUFFER_SLOT_COUNT];
    memcpy(out_data, slot->data, slot->size);
    *out_size = slot->size;

    atomic_store_explicit(&ring_buffer->data->read_index, read_index + 1, memory_order_release);
    return 0;
}
