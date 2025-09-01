cmake_minimum_required(VERSION 3.24)
project(crobots)

set(BINARY_DIR ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${BINARY_DIR})
make_directory(${BINARY_DIR})

configure_file(LICENSE ${BINARY_DIR} COPYONLY)
configure_file(README.md ${BINARY_DIR} COPYONLY)
configure_file(fonts/RasterForgeRegular.ttf ${BINARY_DIR} COPYONLY)

add_library(crobots_api
    src/IRobot.cpp
    src/Engine.cpp
    src/Arena.cpp
    src/Shot.cpp
    src/Log.cpp
)
set_target_properties(crobots_api PROPERTIES CXX_STANDARD 23)
set_target_properties(crobots_api PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(crobots_api PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_include_directories(crobots_api PUBLIC include)
target_precompile_headers(crobots_api PUBLIC
    <algorithm>
    <cmath>
    <cstdint>
    <cstring>
    <exception>
    <format>
    <fstream>
    <functional>
    <iterator>
    <limits>
    <memory>
    <string>
    <string_view>
    <unordered_map>
    <vector>
)
if(MSVC)
    target_compile_options(crobots_api PUBLIC /Zc:preprocessor)
endif()
target_link_libraries(crobots_api PRIVATE SDL3::SDL3)

file(GLOB ROBOTS robots/*.cpp)
foreach(PATH ${ROBOTS})
    get_filename_component(NAME ${PATH} NAME_WE)
    add_library(${NAME} MODULE ${PATH})
    set_target_properties(${NAME} PROPERTIES CXX_STANDARD 23)
    target_link_libraries(${NAME} PRIVATE crobots_api)
endforeach()

add_subdirectory(lib)
add_executable(crobots WIN32
    src/App.cpp
    src/Arena.cpp
    src/Camera.cpp
    src/Engine.cpp
    src/Loader.cpp
    src/Main.cpp
    src/Renderer.cpp
    src/Shot.cpp
    src/Timer.cpp
)
set_target_properties(crobots PROPERTIES OUTPUT_NAME "crobots++")
set_target_properties(crobots PROPERTIES CXX_STANDARD 23)
target_link_libraries(crobots PRIVATE crobots_api crobots_lib)

include(cmake/AddVoxModel.cmake)
add_vox_model(models/default)

enable_testing()
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/test)
