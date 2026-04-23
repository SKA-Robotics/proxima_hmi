add_library(audio_lib STATIC
    src/audio_buffer.c
    src/audio_decoder.c
    src/audio_format.c
    src/audio_result.c
)

target_include_directories(audio_lib PUBLIC include)

target_link_libraries(audio_lib
    core_lib
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
)
target_include_directories(audio_lib PUBLIC ${FREERTOS_CONFIG_DIR})