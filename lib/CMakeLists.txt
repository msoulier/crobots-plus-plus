set(GLM_BUILD_LIBRARY OFF)
set(SDLTTF_VENDORED ON)
add_subdirectory(CLI11)
add_subdirectory(SDL)
add_subdirectory(SDL_ttf)
add_subdirectory(SDLx_gpu)
add_subdirectory(SDLx_model)
add_subdirectory(glm)
add_subdirectory(json)
add_subdirectory(stb)

add_library(crobots_lib INTERFACE)
target_link_libraries(crobots_lib INTERFACE CLI11::CLI11)
target_link_libraries(crobots_lib INTERFACE SDL3::SDL3)
target_link_libraries(crobots_lib INTERFACE SDL3_ttf::SDL3_ttf)
target_link_libraries(crobots_lib INTERFACE SDLx_gpu::SDLx_gpu)
target_link_libraries(crobots_lib INTERFACE SDLx_model::SDLx_model)
target_link_libraries(crobots_lib INTERFACE glm)
target_link_libraries(crobots_lib INTERFACE json)
target_link_libraries(crobots_lib INTERFACE stb)
target_precompile_headers(crobots_lib INTERFACE
    <CLI/CLI.hpp>
    <glm/glm.hpp>
)