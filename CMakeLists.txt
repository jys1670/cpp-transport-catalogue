cmake_minimum_required(VERSION 3.21)
project(transport-catalogue CXX)
set(CMAKE_CXX_STANDARD 17)

set(ENABLE_WARNINGS_AS_ERRORS true)
set(ENABLE_STDLIB_DEBUG_MACROS false)
set(ENABLE_SANITIZERS false)
set(SAVE_TEMP_FILES false)

# Multithreading
add_compile_options(-pthread)

set(CMAKE_CXX_FLAGS_DEBUG "-gdwarf-4 -O0 -fno-inline -fdiagnostics-color -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0")

if((ENABLE_WARNINGS_AS_ERRORS) AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
        message("WARNINGS as errors enabled")
        add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

if((ENABLE_STDLIB_DEBUG_MACROS) AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
        message("STDLIB debugging enabled")
        add_compile_definitions("_GLIBCXX_DEBUG _GLIBCXX_DEBUG_PEDANTIC")
endif()

if((ENABLE_SANITIZERS) AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
        message("SANITIZERS enabled")
        set(san_options
                -fsanitize=address
                #-fsanitize=thread
                -fsanitize=leak
                -fno-omit-frame-pointer
                -fno-optimize-sibling-calls
                )
        add_compile_options(${san_options})
        add_link_options(${san_options})
endif()

if(SAVE_TEMP_FILES)
        message("TEMP files saved")
        add_compile_options(-g -masm=intel -save-temps)
endif()

if(NOT CMAKE_BUILD_TYPE)
        message( FATAL_ERROR "Choose a build type" )
endif(NOT CMAKE_BUILD_TYPE)


aux_source_directory(./transport-catalogue SOURCES)
add_executable(${PROJECT_NAME} ${SOURCES})