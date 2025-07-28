cmake_minimum_required(VERSION 3.24)
project(Crobots)

set(BINARY_DIR ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${BINARY_DIR})
make_directory(${BINARY_DIR})

add_subdirectory(lib/SDL)

add_library(IRobot
    src/IRobot.cpp
)
target_include_directories(IRobot PUBLIC include)

file(GLOB ROBOTS robots/*.cpp)
foreach(PATH ${ROBOTS})
    get_filename_component(NAME ${PATH} NAME_WE)
    add_library(${NAME} MODULE ${PATH})
    target_link_libraries(${NAME} PRIVATE IRobot)
endforeach()

add_executable(Crobots WIN32 src/Main.cpp)
set_target_properties(Crobots PROPERTIES OUTPUT_NAME "crobots++")
target_link_libraries(Crobots PRIVATE IRobot SDL3::SDL3)