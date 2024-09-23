cmake_minimum_required(VERSION 3.28)
project(fractals-cl)

set(CMAKE_CXX_STANDARD 17)

add_executable(fractals-cl
        src/main.cpp
        src/cl_helper.cpp
        src/cl_helper.h
        src/fractal_viewer.cpp
        src/fractal_viewer.h
        src/fractals.cpp
        src/fractals.h
        src/imgui/imconfig.h
        src/imgui/imgui.h
        src/imgui/imgui.cpp
        src/imgui/imgui_draw.cpp
        src/imgui/imgui_internal.h
        src/imgui/imgui_tables.cpp
        src/imgui/imgui_widgets.cpp
        src/imgui/imstb_rectpack.h
        src/imgui/imstb_textedit.h
        src/imgui/imstb_truetype.h
        src/imgui/imgui_impl_sdl2.cpp
        src/imgui/imgui_impl_sdl2.h
        src/imgui/imgui_impl_sdlrenderer2.cpp
        src/imgui/imgui_impl_sdlrenderer2.h)

find_package(OpenCL REQUIRED)
target_link_libraries(fractals-cl PRIVATE OpenCL::OpenCL)

find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
target_link_libraries(fractals-cl PRIVATE SDL2::SDL2)