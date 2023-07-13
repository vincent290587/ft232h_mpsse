cmake_minimum_required(VERSION 3.25)
project(libMAX17055 C)

set(CMAKE_C_STANDARD 11)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(${PROJECT_NAME}
        MAX17055.c
        )

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        )

