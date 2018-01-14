cmake_minimum_required(VERSION 3.7)
project(Img2Spec)

set(CMAKE_CXX_STANDARD 14)

find_package(SDL2 REQUIRED)
find_package(OpenGL REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories("src/imgui" "src/parson" "src/stb" "src/platform")

set(SOURCES
        src/main.cpp

        src/imgui/imconfig.h
        src/imgui/imgui.cpp
        src/imgui/imgui.h
        src/imgui/imgui_draw.cpp
        src/imgui/imgui_impl_sdl.cpp
        src/imgui/imgui_impl_sdl.h
        src/imgui/imgui_internal.h
        src/imgui/imgui_user.inl

        src/parson/parson.c
        src/parson/parson.h

        src/device.h
        src/modifier.h

        src/zxspectrumdevice.h
        src/zx3x64device.h
        src/zxhalftiledevice.h
        src/c64hiresdevice.h
        src/c64multicolordevice.h

        src/scaleposmodifier.h
        src/rgbmodifier.h
        src/yiqmodifier.h
        src/hsvmodifier.h
        src/noisemodifier.h
        src/blurmodifier.h
        src/edgemodifier.h
        src/quantizemodifier.h
        src/minmaxmodifier.h
        src/ordereddithermodifier.h
        src/errordiffusiondithermodifier.h
        src/contrastmodifier.h
        src/superblackmodifier.h
        src/curvemodifier.h

        src/platform/common.h

        )

if (WIN32)
    set(SOURCES ${SOURCES}
            src/platform/win32.cpp
            src/platform/win32.h)
else ()
    set(SOURCES ${SOURCES}
            src/platform/nonwin32.cpp
            src/platform/nonwin32.h
            src/platform/noc_file_dialog.h)
    if (APPLE)
        add_definitions(-DNOC_FILE_DIALOG_OSX)
        find_library(APPKIT AppKit)
        set_source_files_properties(src/platform/nonwin32.cpp
                PROPERTIES LANGUAGE C COMPILE_FLAGS "-x objective-c")
    else ()
        add_definitions(-DNOC_FILE_DIALOG_GTK)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
        include_directories(${GTK3_INCLUDE_DIRS})
        link_directories(${GTK3_LIBRARY_DIRS})
        add_definitions(${GTK3_CFLAGS_OTHER})
    endif ()
endif ()

add_definitions(-DIMGUI_INCLUDE_IMGUI_USER_INL)

add_executable(img2spec WIN32 ${SOURCES})
target_link_libraries(img2spec ${SDL2_LIBRARIES} ${OPENGL_LIBRARIES})

if (WIN32)
    target_link_libraries(img2spec winmm imm32 version)
endif ()

if (GTK3_FOUND)
    target_link_libraries(img2spec ${GTK3_LIBRARIES})
endif ()

if (APPKIT)
    target_link_libraries(img2spec ${APPKIT})
endif ()
