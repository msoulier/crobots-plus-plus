function(add_shader TARGET PATH)
    set(DEPENDS ${ARGN})
    get_filename_component(NAME ${PATH} NAME)
    get_filename_component(DIRECTORY ${PATH} DIRECTORY)
    set(HLSL ${CMAKE_CURRENT_SOURCE_DIR}/${DIRECTORY}/${NAME})
    set(SPV ${CMAKE_CURRENT_SOURCE_DIR}/${DIRECTORY}/bin/${NAME}.spv)
    set(DXIL ${CMAKE_CURRENT_SOURCE_DIR}/${DIRECTORY}/bin/${NAME}.dxil)
    set(MSL ${CMAKE_CURRENT_SOURCE_DIR}/${DIRECTORY}/bin/${NAME}.msl)
    set(JSON ${CMAKE_CURRENT_SOURCE_DIR}/${DIRECTORY}/bin/${NAME}.json)
    if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
        if(MSVC)
            set(SHADERCROSS ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../lib/SDL_shadercross/msvc/shadercross.exe)
        endif()
        function(compile OUTPUT)
            add_custom_command(
                OUTPUT ${OUTPUT}
                COMMAND ${SHADERCROSS} ${HLSL} -s hlsl -o ${OUTPUT} -I src
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                DEPENDS ${HLSL} ${DEPENDS}
                COMMENT ${OUTPUT}
            )
            get_filename_component(NAME ${OUTPUT} NAME)
            string(REPLACE . _ NAME ${NAME})
            set(NAME compile_${NAME})
            add_custom_target(${NAME} DEPENDS ${OUTPUT})
            add_dependencies(${TARGET} ${NAME})
        endfunction()
        if (MSVC)
            compile(${SPV})
            compile(${DXIL})
            compile(${MSL})
            compile(${JSON})
        endif()
    endif()
    function(package OUTPUT)
        get_filename_component(NAME ${OUTPUT} NAME)
        set(BINARY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${NAME})
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