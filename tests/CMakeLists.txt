if (ENABLE_TESTING)
    find_package (Boost COMPONENTS unit_test_framework REQUIRED)
    include_directories(${Boost_INCLUDE_DIRS})

    set(TEST_MAIN "unit_tests")
    aux_source_directory(../transport-catalogue TEST_SOURCES)
    list(REMOVE_ITEM TEST_SOURCES "../transport-catalogue/main.cpp")
    set(TEST_SOURCES "${TEST_SOURCES}" ./test.cpp)

    protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

    add_executable(${TEST_MAIN} ${PROTO_SRCS} ${PROTO_HDRS} ${TEST_SOURCES})

    target_include_directories(${TEST_MAIN}
            PUBLIC ${Protobuf_INCLUDE_DIRS}
            PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
            )
    target_link_libraries(${TEST_MAIN}
            PUBLIC "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>"
            PUBLIC ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY}
            )

    add_debug_compiler_options(
            TARGET ${TEST_MAIN}
            "WARNINGS_AS_ERRORS" ${WARNINGS_AS_ERRORS}
            "SANITIZERS" ${SANITIZERS}
            "SAVE_TEMP_FILES" ${SAVE_TEMP_FILES}
    )

    target_compile_definitions(${TEST_MAIN} PRIVATE CURR_TEST_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

    set_target_properties(
            ${TEST_MAIN}
            PROPERTIES
            ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
            LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )


endif()