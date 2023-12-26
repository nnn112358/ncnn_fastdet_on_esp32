# The following lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.10)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(nanodet)

set(CMAKE_SYSTEM_NAME freertos)
set(CMAKE_SYSTEM_PROCESSOR xtensa-esp32)


set(ncnn_DIR "${CMAKE_CURRENT_SOURCE_DIR}/ncnn/lib/cmake/ncnn")
find_package(ncnn REQUIRED)

target_link_libraries(${CMAKE_PROJECT_NAME}.elf PRIVATE ncnn)

