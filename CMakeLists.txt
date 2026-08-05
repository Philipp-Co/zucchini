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

set(ZUC_PLATFORM "" CACHE STRING "Zielplattform: macos oder linux (leer = automatisch anhand des Systems erkannt)")
set_property(CACHE ZUC_PLATFORM PROPERTY STRINGS "" "macos" "linux")

if(NOT ZUC_PLATFORM)
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(ZUC_PLATFORM "macos")
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(ZUC_PLATFORM "linux")
    else()
        message(FATAL_ERROR "Nicht unterstuetzte Plattform '${CMAKE_SYSTEM_NAME}'. Explizit setzen mit -DZUC_PLATFORM=macos oder -DZUC_PLATFORM=linux")
    endif()
endif()

if(NOT ZUC_PLATFORM STREQUAL "macos" AND NOT ZUC_PLATFORM STREQUAL "linux")
    message(FATAL_ERROR "ZUC_PLATFORM muss 'macos' oder 'linux' sein, ist aber '${ZUC_PLATFORM}'")
endif()

message(STATUS "ZUC_PLATFORM = ${ZUC_PLATFORM}")

add_subdirectory(utils)
add_subdirectory(server)
add_subdirectory(admin_client)
add_subdirectory(api)
add_subdirectory(test)
