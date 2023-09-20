CMAKE_MINIMUM_REQUIRED(VERSION 3.20)
PROJECT(GeoTiffUtils)

SET(CMAKE_CXX_STANDARD 17)

OPTION(GEO_TIFF_UTILS_WITH_EXAMPLE "Build GeoTiffUtils Example" ON)

SET(tiff-tools OFF CACHE BOOL "build TIFF tools" FORCE)
SET(tiff-tests OFF CACHE BOOL "build TIFF tests" FORCE)
SET(tiff-contrib OFF CACHE BOOL "build TIFF contrib" FORCE)
SET(tiff-docs OFF CACHE BOOL "build TIFF documentation" FORCE)
SET(tiff-deprecated OFF CACHE BOOL "build TIFF deprecated features" FORCE)

SET(WEBP_BUILD_ANIM_UTILS OFF CACHE BOOL "Build animation utilities." FORCE)
SET(WEBP_BUILD_CWEBP OFF CACHE BOOL "Build the cwebp command line tool." FORCE)
SET(WEBP_BUILD_DWEBP OFF CACHE BOOL "Build the dwebp command line tool." FORCE)
SET(WEBP_BUILD_GIF2WEBP OFF CACHE BOOL "Build the gif2webp conversion tool." FORCE)
SET(WEBP_BUILD_IMG2WEBP OFF CACHE BOOL "Build the img2webp animation tool." FORCE)
SET(WEBP_BUILD_VWEBP OFF CACHE BOOL "Build the vwebp viewer tool." FORCE)
SET(WEBP_BUILD_WEBPINFO OFF CACHE BOOL "Build the webpinfo command line tool." FORCE)
SET(WEBP_BUILD_LIBWEBPMUX OFF CACHE BOOL "Build the libwebpmux library." FORCE)
SET(WEBP_BUILD_WEBPMUX OFF CACHE BOOL "Build the webpmux command line tool." FORCE)
SET(WEBP_BUILD_EXTRAS OFF CACHE BOOL "Build extras." FORCE)

ADD_SUBDIRECTORY(libtiff)
ADD_SUBDIRECTORY(libwebp)

ADD_LIBRARY(GeoTiffUtils
            TiffImage.cpp
            TiffImage.h
            WebPImage.cpp
            WebPImage.h)

TARGET_INCLUDE_DIRECTORIES(GeoTiffUtils PUBLIC libtiff/libtiff)
TARGET_INCLUDE_DIRECTORIES(GeoTiffUtils PUBLIC libwebp/src)

TARGET_LINK_LIBRARIES(GeoTiffUtils tiff)
TARGET_LINK_LIBRARIES(GeoTiffUtils webp)

IF (GEO_TIFF_UTILS_WITH_EXAMPLE)
    MESSAGE("GeoTiffUtils: Building Example")
    ADD_EXECUTABLE(GeoTiffTester main.cpp)
    TARGET_LINK_LIBRARIES(GeoTiffTester GeoTiffUtils)
ENDIF ()
