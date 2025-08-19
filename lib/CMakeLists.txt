set(GLM_BUILD_LIBRARY OFF)
add_subdirectory(CLI11)
add_subdirectory(SDL)
add_subdirectory(SDLx_gpu)
add_subdirectory(glm)

add_library(crobots_lib INTERFACE)
target_link_libraries(crobots_lib INTERFACE CLI11::CLI11)
target_link_libraries(crobots_lib INTERFACE SDL3::SDL3)
target_link_libraries(crobots_lib INTERFACE SDLx_gpu::SDLx_gpu)
target_link_libraries(crobots_lib INTERFACE glm)
target_precompile_headers(crobots_lib INTERFACE
    <CLI/CLI.hpp>
    <glm/glm.hpp>
)