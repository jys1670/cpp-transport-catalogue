aux_source_directory(./ APP_SOURCES)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

add_executable(${EXECUTABLE_NAME} ${PROTO_SRCS} ${PROTO_HDRS} ${APP_SOURCES})

target_include_directories(${EXECUTABLE_NAME}
        PUBLIC ${Protobuf_INCLUDE_DIRS}
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
        )
target_link_libraries(${EXECUTABLE_NAME}
        PUBLIC "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>"
        )

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


if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
    set_target_properties(
            ${EXECUTABLE_NAME}
            PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY "$<1:${CMAKE_BINARY_DIR}/bin>"
            RUNTIME_OUTPUT_DIRECTORY_DEBUG "$<1:${CMAKE_BINARY_DIR}/bin>"
            RUNTIME_OUTPUT_DIRECTORY_RELEASE "$<1:${CMAKE_BINARY_DIR}/bin>"
            LIBRARY_OUTPUT_DIRECTORY "$<1:${CMAKE_BINARY_DIR}/lib>"
            LIBRARY_OUTPUT_DIRECTORY_DEBUG "$<1:${CMAKE_BINARY_DIR}/lib>"
            LIBRARY_OUTPUT_DIRECTORY_RELEASE "$<1:${CMAKE_BINARY_DIR}/lib>"
            ARCHIVE_OUTPUT_DIRECTORY "$<1:${CMAKE_BINARY_DIR}/lib>"
            ARCHIVE_OUTPUT_DIRECTORY_DEBUG "$<1:${CMAKE_BINARY_DIR}/lib>"
            ARCHIVE_OUTPUT_DIRECTORY_RELEASE "$<1:${CMAKE_BINARY_DIR}/lib>"
    )
endif ()