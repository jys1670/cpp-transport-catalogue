
aux_source_directory(./ APP_SOURCES)
add_executable(${EXECUTABLE_NAME} ${APP_SOURCES})

add_debug_compiler_options(
        TARGET ${EXECUTABLE_NAME}
        "WARNINGS_AS_ERRORS" ${WARNINGS_AS_ERRORS}
        "SANITIZERS" ${SANITIZERS}
        "SAVE_TEMP_FILES" ${SAVE_TEMP_FILES}
)

set_target_properties(
        ${EXECUTABLE_NAME}
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
