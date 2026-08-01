add_library(zucchini_client STATIC
    src/client.c
)

target_include_directories(zucchini_client
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(zucchini_client
    PUBLIC
        zucchini_utils
)
