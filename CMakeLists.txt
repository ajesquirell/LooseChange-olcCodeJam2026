cmake_minimum_required(VERSION 3.15)
project(PGEApp)
 
include(FetchContent)

# Force FetchContent to always check for updates
set(FETCHCONTENT_UPDATES_DISCONNECTED OFF)

# Fetch olcPixelGameEngine3.h
FetchContent_Declare(
    olcPixelGameEngine3
    URL "https://raw.githubusercontent.com/OneLoneCoder/olcPixelGameEngine3/develop/olcPixelGameEngine3.h"
    DOWNLOAD_NO_EXTRACT ON
    SOURCE_DIR "${CMAKE_BINARY_DIR}/_deps/olcpixelgameengine3-src"
)

# Fetch the olcUTIL3 utilities.
foreach(UTIL Animate2D Camera2D GameMode Geometry2D QuadTree)
    FetchContent_Declare(
        olcUTIL3_${UTIL}
        URL "https://raw.githubusercontent.com/OneLoneCoder/olcPixelGameEngine3/develop/utilities/olcUTIL3_${UTIL}.h"
        DOWNLOAD_NO_EXTRACT ON
        SOURCE_DIR "${CMAKE_BINARY_DIR}/_deps/olcpixelgameengine3-src/utilities"
    )
    FetchContent_MakeAvailable(olcUTIL3_${UTIL})
endforeach()

# Fetch miniaudio.h
FetchContent_Declare(
    miniaudio
    URL "https://raw.githubusercontent.com/mackron/miniaudio/0.11.25/miniaudio.h"
    DOWNLOAD_NO_EXTRACT ON
    SOURCE_DIR "${CMAKE_BINARY_DIR}/_deps/olcpixelgameengine3-src/extensions/miniaudio"
)

# Fetch olcPGE3_Miniaudio.h
FetchContent_Declare(
    olcPGEX3_Miniaudio
    URL "https://raw.githubusercontent.com/OneLoneCoder/olcPixelGameEngine3/develop/extensions/miniaudio/olcPGEX3_Miniaudio.h"
    DOWNLOAD_NO_EXTRACT ON
    SOURCE_DIR "${CMAKE_BINARY_DIR}/_deps/olcpixelgameengine3-src/extensions/miniaudio"
)

# Make them available (download, etc)
FetchContent_MakeAvailable(miniaudio olcPGEX3_Miniaudio olcPixelGameEngine3)

# Add them to the include paths
include_directories(
    "${CMAKE_BINARY_DIR}/_deps/olcpixelgameengine3-src/extensions/miniaudio"
    "${CMAKE_BINARY_DIR}/_deps/olcpixelgameengine3-src"
)

# Wrangle Source files
file(
    GLOB SOURCES
    src/*.c
    src/*.cpp
    src/*.h
    src/*.hpp
)

# Include helper scripts
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/Helpers.cmake)

# platform detection and preparation
pge3_prepare_platforms()

# add_executable with all of the pge3 batteries includes
pge3_add_program(${CMAKE_PROJECT_NAME} ${SOURCES})

# copy/manage assets directory
pge3_handle_assets(${CMAKE_PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/assets)
