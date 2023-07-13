cmake_minimum_required(VERSION 3.25)
project(ft232h)

set(CMAKE_CXX_STANDARD 17)

# Have cmake create a compile database
set(CMAKE_EXPORT_COMPILE_COMMANDS				ON)

include(FetchContent)
FetchContent_Declare(
        libDRV2605
        GIT_REPOSITORY https://github.com/vincent290587/dr2605_mpsse.git
        GIT_TAG        origin/develop
        SOURCE_DIR     ${CMAKE_CURRENT_SOURCE_DIR}/libDRV2605
)
FetchContent_MakeAvailable(libDRV2605)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_definitions(-DFTDIMPSSE_STATIC) # static lib
#add_definitions(-DINFRA_DEBUG_ENABLE)

add_subdirectory(./libMPSSE_1.0.3)

add_executable(${PROJECT_NAME}_gpio
        FT232H_GPIO.cpp
        )

add_executable(${PROJECT_NAME}_drv2605
        FT232H_DRV.cpp
        )

add_executable(${PROJECT_NAME}_bme280
        FT232H_BME280.c
        )

target_include_directories(libDRV2605 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME}_gpio FTDI_MPSSE_Lib libDRV2605)
target_link_libraries(${PROJECT_NAME}_drv2605 FTDI_MPSSE_Lib libDRV2605)
target_link_libraries(${PROJECT_NAME}_bme280 FTDI_MPSSE_Lib libDRV2605)

file( GENERATE OUTPUT "${CMAKE_BINARY_DIR}/.gitignore" CONTENT "*" )
