# Set min req version of Cmake
cmake_minimum_required(VERSION 3.20)

# Set the project name
project(FTDI_MPSSE_Lib)

# Set our include directories
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/libftd2xx
)

# Set our source files
set(LIB_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ftdi_i2c.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ftdi_infra.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ftdi_mid.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ftdi_spi.c
        ${CMAKE_CURRENT_SOURCE_DIR}/src/memcpy.c
)

# Set the lib output folder
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../lib)

# Create our static library
add_library(${PROJECT_NAME} STATIC ${LIB_SRC})

link_libraries(-lpthread -lrt -ldl)

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/libftd2xx
        )
