cmake_minimum_required(VERSION 3.24)
project(SDLx_model)

add_library(SDLx_model
    src/SDL_model.cpp
    src/internal.cpp
    src/stb_image.c
    src/tiny_obj_loader.cpp
    src/vox_obj.cpp
    src/vox_raw.cpp
)
target_include_directories(SDLx_model PUBLIC include)
target_link_libraries(SDLx_model PRIVATE SDL3::SDL3)
set_target_properties(SDLx_model PROPERTIES CXX_STANDARD 23)
set_target_properties(SDLx_model PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
add_library(SDLx_model::SDLx_model ALIAS SDLx_model)