add_executable(hmi main.c)

pico_set_program_name(hmi "Proxima HMI")
pico_set_program_version(hmi "0.1")

pico_enable_stdio_uart(hmi 0)
pico_enable_stdio_usb(hmi 1)

target_link_libraries(hmi PUBLIC
    common_lib
    pico_stdlib
    bsp
    lvgl
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
)

target_include_directories(hmi PRIVATE ${CMAKE_CURRENT_LIST_DIR})

pico_add_extra_outputs(hmi)

set(HMI_DIST_DIR "${CMAKE_SOURCE_DIR}/dist")
add_custom_command(
    TARGET hmi POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${HMI_DIST_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE_DIR:hmi>/hmi.uf2
        ${HMI_DIST_DIR}/hmi.uf2
    COMMENT "Copy UF2 to dist/"
    VERBATIM
)