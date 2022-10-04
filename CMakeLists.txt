cmake_minimum_required(VERSION 3.21)
project("transport-catalogue" LANGUAGES CXX)

### Global variables
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

### Options
# Tests
option(ENABLE_TESTING "Whether to enable unit tests" OFF)

# Debug build compilation options
option(WARNINGS_AS_ERRORS "Warnings as errors in Debug builds" OFF)
option(SANITIZERS "Whether to enable sanitizers in Debug builds" OFF)
option(SAVE_TEMP_FILES "Whether save compiler temp files in Debug builds" OFF)

# Linking
option(STATIC_LINKING "Prefer static linking over dynamic" ON)

### Project Name
set(EXECUTABLE_NAME "main")

### Cmake modules
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/)
include(CompileOptions)
include(Docs)

# checks
if (NOT CMAKE_BUILD_TYPE)
        message(STATUS "No build type selected, default to Debug")
        set(CMAKE_BUILD_TYPE "Debug")
endif()

if(STATIC_LINKING)
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".lib")
else()
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".so" ".dll")
endif()

find_package(Protobuf REQUIRED)
file(GLOB PROTO_FILES ./proto/*.proto)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

### Subdirs
add_subdirectory(transport-catalogue)
add_subdirectory(tests)
