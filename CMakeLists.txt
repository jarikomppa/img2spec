cmake_minimum_required(VERSION 3.7)
project(Img2Spec)

set(CMAKE_CXX_STANDARD 14)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories("src/imgui" "src/parson" "src/stb")

set(SOURCES
        src/main.cpp
        src/imgui/imgui.cpp
        src/imgui/imgui_draw.cpp
        src/imgui/imgui_impl_sdl.cpp
        src/parson/parson.c
        )

add_definitions(-DIMGUI_INCLUDE_IMGUI_USER_INL)

add_executable(img2spec WIN32 ${SOURCES})
target_link_libraries(img2spec ${SDL2_LIBRARIES})

if (WIN32)
    target_link_libraries(img2spec opengl32 winmm imm32 version)
endif ()
