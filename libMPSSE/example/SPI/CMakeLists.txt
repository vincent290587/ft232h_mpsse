# Set min req version of Cmake
cmake_minimum_required(VERSION 3.16)

# Set the project name
project(FTDI_MPSSE_Lib)

# Set our include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../inc
                    ${CMAKE_CURRENT_SOURCE_DIR}/../../inc/linux)

# Set our source files
set(SAMPLE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/sample_spi.c
)

# Set the lib output folderj
set(EXECUTABLE_OUTPUT_PATH ../output)

# Set debug compile options
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-ggdb")

# Create our static library
add_executable(sample_spi ${SAMPLE_SRC})

# Link against our MPSSE library
target_link_libraries(sample_spi PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/libMPSSE.a
                        dl
)