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

add_library(crobots_api src/IRobot.cpp)
set_target_properties(crobots_api PROPERTIES CXX_STANDARD 23)
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

file(GLOB ROBOTS robots/*.cpp)
foreach(PATH ${ROBOTS})
    get_filename_component(NAME ${PATH} NAME_WE)
    add_library(${NAME} MODULE ${PATH})
    target_link_libraries(${NAME} PRIVATE crobots_api)
endforeach()

add_subdirectory(lib)
add_executable(crobots WIN32
    src/App.cpp
    src/Arena.cpp
    src/Camera.cpp
    src/DebugGroup.cpp
    src/Engine.cpp
    src/Loader.cpp
    src/Log.cpp
    src/Main.cpp
    src/Math.cpp
    src/Mesh.cpp
    src/Model.cpp
    src/ModelVoxObj.cpp
    src/ParticleBuffer.cpp
    src/Pipeline.cpp
    src/Renderer.cpp
    src/Shader.cpp
    src/Texture.cpp
    src/Timer.cpp
    src/Window.cpp
)
set_target_properties(crobots PROPERTIES OUTPUT_NAME "crobots++")
set_target_properties(crobots PROPERTIES CXX_STANDARD 23)
target_link_libraries(crobots PRIVATE crobots_api crobots_lib)

include(cmake/AddShader.cmake)
add_shader(crobots ModelVoxObj.frag)
add_shader(crobots ModelVoxObj.vert)
add_shader(crobots Particle.comp ${CMAKE_SOURCE_DIR}/src/ShaderConfig.h)
add_shader(crobots Particle.frag)
add_shader(crobots Particle.vert)

include(cmake/AddModel.cmake)
add_model(crobots default)
