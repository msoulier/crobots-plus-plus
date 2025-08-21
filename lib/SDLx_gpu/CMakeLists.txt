cmake_minimum_required(VERSION 3.24)
project(SDLx_gpu)

add_library(SDLx_gpu src/SDL_gpu.cpp)
target_include_directories(SDLx_gpu PUBLIC include)
target_link_libraries(SDLx_gpu PUBLIC
    SDL3::SDL3
    SDL3_ttf::SDL3_ttf
    SDLx_model::SDLx_model
    stb json
)
set_target_properties(SDLx_gpu PROPERTIES CXX_STANDARD 23)
set_target_properties(SDLx_gpu PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
add_library(SDLx_gpu::SDLx_gpu ALIAS SDLx_gpu)

include(${CMAKE_SOURCE_DIR}/cmake/AddShader.cmake)
add_shader(SDLx_gpu shaders/line_2d.frag)
add_shader(SDLx_gpu shaders/line_2d.vert shaders/shader.hlsl)
add_shader(SDLx_gpu shaders/line_3d.frag)
add_shader(SDLx_gpu shaders/line_3d.vert shaders/shader.hlsl)
add_shader(SDLx_gpu shaders/text_2d.frag)
add_shader(SDLx_gpu shaders/text_2d.vert shaders/shader.hlsl)
add_shader(SDLx_gpu shaders/vox_obj.frag)
add_shader(SDLx_gpu shaders/vox_obj.vert)
add_shader(SDLx_gpu shaders/vox_raw.frag)
add_shader(SDLx_gpu shaders/vox_raw.vert shaders/shader.hlsl)