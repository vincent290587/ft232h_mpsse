cmake_minimum_required(VERSION 3.25)
project(libLSM6DS3TR C)

set(CMAKE_C_STANDARD 11)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(${PROJECT_NAME}
        lsm6ds3_wrapper.c
        lsm6ds3tr-c_reg.c
        )

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

