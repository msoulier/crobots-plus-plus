function(add_shader TARGET FILE)
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
        add_dependencies(${TARGET} ${NAME})
    endfunction()
    if (MSVC)
        set(SHADERCROSS ${CMAKE_SOURCE_DIR}/lib/SDL_shadercross/msvc/shadercross.exe)
        compile(glslc ${GLSL} ${SPV} -g)
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
        add_dependencies(${TARGET} ${NAME})
    endfunction()
    package(${SPV})
    if(WIN32)
        package(${DXIL})
    elseif(APPLE)
        package(${MSL})
    endif()
    package(${JSON})
endfunction()