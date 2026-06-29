cmake_minimum_required(VERSION 3.20)

project(file_processor C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(file_processor
        src/main.c
        src/util.h
        src/platform_directory/platform_directory.h
        src/report.c
        src/report.h
        src/platform_threading/platform_thread.h
)

target_include_directories(file_processor PRIVATE
    src
)

if (WIN32)
    target_sources(file_processor PRIVATE
            src/platform_directory/platform_dir_win32.c
            src/platform_threading/platform_threading_win32.c
    )
else ()

endif ()