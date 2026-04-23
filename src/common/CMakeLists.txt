add_library(common_lib INTERFACE) 

target_include_directories(common_lib INTERFACE include)

target_link_libraries(common_lib INTERFACE 
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
)
target_include_directories(common_lib INTERFACE ${FREERTOS_CONFIG_DIR})

add_subdirectory(core)
add_subdirectory(audio)

target_link_libraries(common_lib INTERFACE
    core_lib
    audio_lib
)