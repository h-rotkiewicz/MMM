# see https://alexreinking.com/blog/how-to-use-cmake-without-the-agonizing-pain-part-1.html
include(ExternalProject)
cmake_minimum_required(VERSION 3.29)
set(CMAKE_CXX_STANDARD 20)
set(TGUI_BACKEND SFML_GRAPHICS)
set(TGUI_CXX_STANDARD 17)
project(MMM)


if(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
  find_package(Catch2 3 OPTIONAL)
  add_subdirectory(Tests)
endif()

add_subdirectory(${CMAKE_SOURCE_DIR}/external/SFML)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/TGUI)

file(GLOB SOURCES "src/*.cpp" "src/*.c")
include_directories(${CMAKE_SOURCE_DIR}/external/TGUI/include)
include_directories(${CMAKE_SOURCE_DIR}/external/SFML/include)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} tgui sfml-graphics sfml-window )
