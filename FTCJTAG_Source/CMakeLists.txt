# Set min req version of Cmake
cmake_minimum_required(VERSION 3.27)

# Set the project name
project(FCJTAG_Lib VERSION 2.0.0)

# Set our include directories
include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Set our source files
set(LIB_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/FT2232h.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FT2232hMpsseJtag.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FTCJTAG.cpp
)

# Set the lib output folder
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../lib)

# Create our static library
add_library(${PROJECT_NAME} STATIC ${LIB_SRC})

link_libraries(-lpthread -lrt -ldl)

target_link_libraries(${PROJECT_NAME} PUBLIC FTDI_MPSSE_Lib)

target_include_directories(${PROJECT_NAME} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
