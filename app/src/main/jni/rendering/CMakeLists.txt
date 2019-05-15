cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -Wall")
add_definitions("-DGLM_FORCE_SIZE_T_LENGTH -DGLM_FORCE_RADIANS")

add_library(render SHARED
        shader.cpp)

target_link_libraries(render
        android
        native_app_glue
        EGL
        GLESv3
        log)