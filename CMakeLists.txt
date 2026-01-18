cmake_minimum_required(VERSION 4.1)
project(wiimote_audio)

set(CMAKE_CXX_STANDARD 23)
include(FetchContent)
FetchContent_Declare(
        nwlib
        GIT_REPOSITORY https://github.com/capitalistspz/nwlib
        GIT_TAG 5e2fd5f3a8e5fe6d5e8a1535a8d04d8182f876ab
)
FetchContent_MakeAvailable(nwlib)

add_executable(wiimote_audio main.cpp
        stb_vorbis.c)

target_link_libraries(wiimote_audio PRIVATE nwlib)

wut_create_rpx(wiimote_audio)