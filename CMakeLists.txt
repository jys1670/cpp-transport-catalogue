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
option(WARNINGS_AS_ERRORS "Warnings as errors in Debug builds" ON)
option(SANITIZERS "Whether to enable sanitizers" OFF)
option(SAVE_TEMP_FILES "Whether save compiler temp files" OFF)

### Project Name
set(EXECUTABLE_NAME "main")


### Cmake modules
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/)
include(CompileOptions)

# build type check
if (NOT CMAKE_BUILD_TYPE)
        message(STATUS "No build type selected, default to Debug")
        set(CMAKE_BUILD_TYPE "Debug")
endif()

### Subdirs
add_subdirectory(transport-catalogue)
add_subdirectory(tests)

# Install targets
install(TARGETS ${EXECUTABLE_NAME}
        RUNTIME DESTINATION bin)