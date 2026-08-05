add_library(zucchini_api STATIC
    src/api.c
)

target_include_directories(zucchini_api
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(zucchini_api
    PUBLIC
        zucchini_utils
)
