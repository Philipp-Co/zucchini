add_executable(zucchini_admin_client
    src/main.c
    src/hex.c
    src/notify.c
    src/commands/whitelist_add.c
    src/commands/whitelist_remove.c
    src/commands/status.c
    src/commands/help.c
    src/commands/table.c
)

target_include_directories(zucchini_admin_client
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src/include
)

target_link_libraries(zucchini_admin_client
    PUBLIC
        zucchini_utils
)
