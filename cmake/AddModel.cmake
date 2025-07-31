function(add_model TARGET NAME)
    configure_file(${CMAKE_SOURCE_DIR}/models/${NAME}.obj ${BINARY_DIR} COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/models/${NAME}.png ${BINARY_DIR} COPYONLY)
endfunction()