cmake_minimum_required(VERSION 3.26)
project(GeoTiffUtils)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(libtiff)
add_subdirectory(libwebp)

add_library(GeoTiffUtils
        TiffImage.cpp
        TiffImage.h
        WebPImage.cpp
        WebPImage.h)

target_include_directories(GeoTiffUtils PUBLIC libtiff/libtiff)
target_include_directories(GeoTiffUtils PUBLIC libwebp/src)

target_link_libraries(GeoTiffUtils tiff)
target_link_libraries(GeoTiffUtils webp)

add_executable(GeoTiffTester main.cpp)
target_link_libraries(GeoTiffTester GeoTiffUtils)