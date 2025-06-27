cmake_minimum_required(VERSION 3.31)
project(w3dhub_gltf_to_w3d)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SDL3_DIR vendor/SDL3/cmake)
find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)

set(SOURCES
        main.cpp
)

set(IMGUI_SOURCES
        vendor/imgui/imgui.cpp
        vendor/imgui/imgui_demo.cpp
        vendor/imgui/imgui_draw.cpp
        vendor/imgui/imgui_tables.cpp
        vendor/imgui/imgui_widgets.cpp

        vendor/imgui/backends/imgui_impl_sdl3.cpp
        vendor/imgui/backends/imgui_impl_sdlgpu3.cpp)

set(WWLIB_SOURCES
        vendor/wwlib/chunkio.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES} ${WWLIB_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE vendor/wwlib)
target_include_directories(${PROJECT_NAME} PRIVATE vendor/tinygltf)
target_include_directories(${PROJECT_NAME} PRIVATE vendor/imgui)
target_include_directories(${PROJECT_NAME} PRIVATE vendor/imgui/backends)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3)
