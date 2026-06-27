# SPDX-License-Identifier: MIT
#
# SPDX-FileCopyrightText: Copyright (c) 2026 Otonashi Horobu

message(STATUS "[Zephyrus ACR] Downloading CPM...")
include("${PROJECT_SOURCE_DIR}/cmake/get-cpm.cmake")

function(setup_dependencies)
    CPMAddPackage("gh:TheLartians/Ccache.cmake@1.2.5")
    CPMAddPackage("gh:vector-of-bool/cmrc@2.0.1#952ffddba731fc110bd50409e8d2b8a06abbd237")
    CPMAddPackage("gh:gabime/spdlog@1.17.0")
    CPMAddPackage("gh:fmtlib/fmt@12.2.0#12.2.0")
    CPMAddPackage("gh:boost-ext/ut@2.3.1")

    CPMAddPackage(
        URI "gh:madler/zlib@1.3.2"
        OPTIONS
            "ZLIB_BUILD_TESTING OFF"
            "ZLIB_BUILD_SHARED OFF"
    )
    set(ZLIB_ROOT "${PROJECT_BINARY_DIR}/_deps/zlib-src")
    if(zlib_ADDED)
        if(NOT TARGET ZLIB::ZLIB)
            add_library(ZLIB::ZLIB ALIAS zlibstatic)
        endif()
    endif()

    CPMAddPackage(
        URI "gh:pnggroup/libpng@1.8.0#libpng18"
        OPTIONS
            "PNG_TESTS OFF"
            "PNG_TOOLS OFF"
            "PNG_SHARED OFF"
    )
    set(PNG_ROOT "${PROJECT_BINARY_DIR}/_deps/libpng-src")
    if(png_ADDED)
        if(NOT TARGET PNG::PNG)
            add_library(PNG::PNG ALIAS png_static)
        endif()
    endif()

    CPMAddPackage("https://gitlab.com/bzip2/bzip2.git@1.0.8#bzip2-1.0.8")
    set(BZip2_ROOT "${PROJECT_BINARY_DIR}/_deps/bzip2-src")

    CPMAddPackage("gh:harfbuzz/harfbuzz@14.2.1#14.2.1")
    set(HarfBuzz_ROOT "${PROJECT_BINARY_DIR}/_deps/harfbuzz-src")
    if(harfbuzz_ADDED)
        if(NOT TARGET HarfBuzz::HarfBuzz)
            add_library(HarfBuzz::HarfBuzz ALIAS harfbuzz)
        endif()
    endif()

    CPMAddPackage(
        URI "gh:freetype/freetype@2.14.3#VER-2-14-3"
        OPTIONS "FT_DYNAMIC_HARFBUZZ FALSE"
    )
    if(freetype_ADDED)
        if(NOT TARGET Freetype::Freetype)
            add_library(Freetype::Freetype ALIAS freetype)
        endif()
    endif()
    CPMAddPackage(
        URI "gh:mikke89/RmlUi@6.2#6.2"
    )
endfunction()
