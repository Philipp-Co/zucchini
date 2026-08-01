add_library(zucchini_utils STATIC
    src/macos/ipc/key.c
    src/macos/ipc/pmutex.c
    src/macos/ipc/queue.c
    src/macos/ipc/shared_memory.c
    src/ipc/ring_buffer.c
    src/status/status.c
)

target_include_directories(zucchini_utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
