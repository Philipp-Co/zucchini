find_package(Threads REQUIRED)

if(ZUC_PLATFORM STREQUAL "macos")
    set(ZUC_EVENT_HANDLING_SOURCE src/macos/event_handling/event_handling.c)
else()
    set(ZUC_EVENT_HANDLING_SOURCE src/linux/event_handling/event_handling.c)
endif()

add_executable(zucchini_server
    src/main.c
    src/admin/admin.c
    src/server.c
    src/macos/io/networkung/udp.c
    src/client/client.c
    ${ZUC_EVENT_HANDLING_SOURCE}
)

target_include_directories(zucchini_server
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/src/include
)

target_link_libraries(zucchini_server
    PUBLIC
        zucchini_utils
)

set_source_files_properties(
    src/macos/io/networkung/udp.c
    src/macos/io/networkung/tcp.c
    PROPERTIES COMPILE_OPTIONS "-O2"
)

include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
if(ipo_supported)
    set_target_properties(zucchini_server PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(WARNING "LTO nicht unterstützt: ${ipo_error}")
endif()
