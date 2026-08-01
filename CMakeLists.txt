cmake_minimum_required(VERSION 3.16)

project(zucchini
    LANGUAGES C
)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Os)
endif()

add_subdirectory(utils)
add_subdirectory(server)
add_subdirectory(admin_client)
add_subdirectory(client)
add_subdirectory(test)
