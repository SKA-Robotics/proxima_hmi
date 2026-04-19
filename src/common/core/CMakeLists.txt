add_library(core_lib STATIC 
    src/common.c
)

target_include_directories(core_lib PUBLIC include)

target_link_libraries(core_lib
    FreeRTOS-Kernel
    FreeRTOS-Kernel-Heap4
)
target_include_directories(core_lib PUBLIC ${FREERTOS_CONFIG_DIR})