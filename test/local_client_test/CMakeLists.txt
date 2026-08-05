add_executable(zucchini_local_client_test
    src/main.c
)

target_link_libraries(zucchini_local_client_test
    PUBLIC
        zucchini_api
)
