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
target_include_directories(crobots_api PUBLIC include)

file(GLOB ROBOTS robots/*.cpp)
foreach(PATH ${ROBOTS})
    get_filename_component(NAME ${PATH} NAME_WE)
    add_library(${NAME} MODULE ${PATH})
    target_link_libraries(${NAME} PRIVATE crobots_api)
endforeach()

add_subdirectory(lib/SDL)
add_subdirectory(lib/glm)
add_subdirectory(lib/tinyobjloader)
add_subdirectory(lib/json)
add_executable(crobots WIN32
    src/DebugGroup.cpp
    src/Main.cpp
    src/Renderer.cpp
    src/Shader.cpp
    src/Window.cpp
)
set_target_properties(crobots PROPERTIES OUTPUT_NAME "crobots++")
set_target_properties(crobots PROPERTIES CXX_STANDARD 23)
target_link_libraries(crobots PRIVATE
    crobots_api
    SDL3::SDL3
    glm-header-only
    nlohmann_json
    tinyobjloader
)
target_precompile_headers(crobots PRIVATE
    <algorithm>
    <cstdint>
    <exception>
    <format>
    <fstream>
    <glm/glm.hpp>
    <iterator>
    <nlohmann/json.hpp>
    <string>
    <string_view>
    <tiny_obj_loader.h>
    <vector>
)

function(add_shader FILE)
    set(DEPENDS ${ARGN})
    set(GLSL ${CMAKE_SOURCE_DIR}/shaders/${FILE})
    set(SPV ${CMAKE_SOURCE_DIR}/shaders/bin/${FILE}.spv)
    set(DXIL ${CMAKE_SOURCE_DIR}/shaders/bin/${FILE}.dxil)
    set(MSL ${CMAKE_SOURCE_DIR}/shaders/bin/${FILE}.msl)
    set(JSON ${CMAKE_SOURCE_DIR}/shaders/bin/${FILE}.json)
    function(compile PROGRAM SOURCE OUTPUT)
        add_custom_command(
            OUTPUT ${OUTPUT}
            COMMAND ${PROGRAM} ${SOURCE} -o ${OUTPUT} -I src
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            DEPENDS ${SOURCE} ${DEPENDS}
            COMMENT ${OUTPUT}
        )
        get_filename_component(NAME ${OUTPUT} NAME)
        string(REPLACE . _ NAME ${NAME})
        set(NAME compile_${NAME})
        add_custom_target(${NAME} DEPENDS ${OUTPUT})
        add_dependencies(crobots ${NAME})
    endfunction()
    if (MSVC)
        set(SHADERCROSS lib/SDL_shadercross/msvc/shadercross.exe)
        compile(glslc ${GLSL} ${SPV})
        compile(${SHADERCROSS} ${SPV} ${DXIL})
        compile(${SHADERCROSS} ${SPV} ${MSL})
        compile(${SHADERCROSS} ${SPV} ${JSON})
    endif()
    function(package OUTPUT)
        get_filename_component(NAME ${OUTPUT} NAME)
        set(BINARY ${BINARY_DIR}/${NAME})
        add_custom_command(
            OUTPUT ${BINARY}
            COMMAND ${CMAKE_COMMAND} -E copy ${OUTPUT} ${BINARY}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            DEPENDS ${OUTPUT}
            COMMENT ${BINARY}
        )
        string(REPLACE . _ NAME ${NAME})
        set(NAME package_${NAME})
        add_custom_target(${NAME} DEPENDS ${BINARY})
        add_dependencies(crobots ${NAME})
    endfunction()
    package(${SPV})
    if(WIN32)
        package(${DXIL})
    elseif(APPLE)
        package(${MSL})
    endif()
    package(${JSON})
endfunction()