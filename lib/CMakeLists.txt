set(GLM_BUILD_LIBRARY OFF)
add_subdirectory(SDL)
add_subdirectory(glm)
add_subdirectory(json)
add_subdirectory(stb)
add_subdirectory(tinyobjloader)

add_library(crobots_lib INTERFACE)
target_link_libraries(crobots_lib INTERFACE SDL3::SDL3)
target_link_libraries(crobots_lib INTERFACE glm)
target_link_libraries(crobots_lib INTERFACE nlohmann_json)
target_link_libraries(crobots_lib INTERFACE stb)
target_link_libraries(crobots_lib INTERFACE tinyobjloader)
target_compile_definitions(crobots_lib INTERFACE GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_compile_definitions(crobots_lib INTERFACE SDL_MAIN_USE_CALLBACKS)
target_precompile_headers(crobots_lib INTERFACE
    <SDL3/SDL.h>
    <glm/glm.hpp>
    <glm/gtc/matrix_transform.hpp>
    <nlohmann/json.hpp>
    <stb_image.h>
    <tiny_obj_loader.h>
)