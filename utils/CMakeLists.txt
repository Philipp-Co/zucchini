add_library(zucchini_utils STATIC
    src/ipc/key.c
    src/ipc/pmutex.c
    src/ipc/queue.c
    src/ipc/shared_memory.c
    src/ipc/ring_buffer.c
    src/status/status.c
    src/logger/logger.c
)

target_include_directories(zucchini_utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
