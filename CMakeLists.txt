cmake_minimum_required(VERSION 3.25)
project(ft232h)

set(CMAKE_CXX_STANDARD 17)

# Have cmake create a compile database
set(CMAKE_EXPORT_COMPILE_COMMANDS				ON)

# libMPSSE - https://github.com/stephendpmurphy/ftdi_libMPSSE.git
include(FetchContent)
FetchContent_Declare(
        libMPSSE
        GIT_REPOSITORY https://github.com/stephendpmurphy/ftdi_libMPSSE.git
        GIT_TAG        origin/main
        SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/libMPSSE
)
FetchContent_MakeAvailable(libMPSSE)

add_executable(${PROJECT_NAME}
        FT232H_GPIO.cpp
        )

target_link_libraries(${PROJECT_NAME} FTDI_MPSSE_Lib)

file( GENERATE OUTPUT "${CMAKE_BINARY_DIR}/.gitignore" CONTENT "*" )
