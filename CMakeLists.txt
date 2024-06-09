# see https://alexreinking.com/blog/how-to-use-cmake-without-the-agonizing-pain-part-1.html
include(ExternalProject)
cmake_minimum_required(VERSION 3.29)
set(CMAKE_CXX_STANDARD 20)

project(MMM)


add_subdirectory(${CMAKE_SOURCE_DIR}/external/SDL)
include_directories(${CMAKE_SOURCE_DIR}/external/imgui)
include_directories(${CMAKE_SOURCE_DIR}/external/imgui/backends)
include_directories(${CMAKE_SOURCE_DIR}/external/implot)

if(TARGET SDL2::SDL2main)
    # It has an implicit dependency on SDL2 functions, so it MUST be added before SDL2::SDL2 (or SDL2::SDL2-static)
    target_link_libraries(mygame PRIVATE SDL3::SDL3main)
endif()

file(GLOB SOURCES "src/*.cpp" "src/*.c" "external/imgui/*.cpp" "external/imgui/backends/imgui_impl_sdl3.cpp" "external/imgui/backends/imgui_impl_sdlrenderer3.cpp" "external/implot/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} SDL3::SDL3)
