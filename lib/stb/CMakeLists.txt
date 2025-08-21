project(stb)
add_library(stb stb_image.c)
target_include_directories(stb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(stb PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)