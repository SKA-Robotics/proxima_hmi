add_subdirectory(bsp)
add_subdirectory(no-OS-FatFS/src)
add_subdirectory(lvgl)

# lv_conf.h; FatFS (ff.h + ffconf.h) for lv_fs_fatfs.c when LV_USE_FS_FATFS
target_include_directories(lvgl SYSTEM PUBLIC
    "${CMAKE_SOURCE_DIR}/src/common/include"
    "${CMAKE_SOURCE_DIR}/lib/no-OS-FatFS/src/ff15/source"
    "${CMAKE_SOURCE_DIR}/lib/no-OS-FatFS/src/include")