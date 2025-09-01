cmake_minimum_required(VERSION 3.24)
function(create_test NAME)
    add_executable(test_${NAME} ${CMAKE_CURRENT_SOURCE_DIR}/test_${NAME}.cpp)
    set_target_properties(test_${NAME} PROPERTIES CXX_STANDARD 23)
    target_include_directories(test_${NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
    target_link_libraries(test_${NAME} crobots_api)
    add_test(NAME test_${NAME}
             COMMAND test_${NAME})
endfunction()

# placeholder for future tests
create_test(hello)
