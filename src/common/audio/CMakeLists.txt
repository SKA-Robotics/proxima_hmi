add_library(audio_lib STATIC
    src/audio_buffer.c
    src/audio_decoder.c
    src/audio_file.c
    src/audio_format.c
    src/audio_result.c
    src/audio_system.c
)

target_include_directories(audio_lib PUBLIC include)

target_link_libraries(audio_lib
    core_lib
    pico_stdlib
    bsp
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
    no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
)
target_include_directories(audio_lib PUBLIC ${FREERTOS_CONFIG_DIR})