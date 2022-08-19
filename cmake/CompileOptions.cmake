function(add_debug_compiler_options)

    # This function is used with debug builds
    if(NOT (CMAKE_BUILD_TYPE STREQUAL "Debug"))
        return()
    endif()

    set(oneValueArgs TARGET WARNINGS_AS_ERRORS SANITIZERS SAVE_TEMP_FILES)
    cmake_parse_arguments(tar_options "" "${oneValueArgs}" "" ${ARGN} )

    set(MSVC_GENERAL_DBG
            /Z7
            /Od
            )
    set(MSVC_WARNINGS
            /W4 # Baseline reasonable warnings
            /w14242 # 'identifier': conversion from 'type1' to 'type1', possible loss of data
            /w14254 # 'operator': conversion from 'type1:field_bits' to 'type2:field_bits', possible loss of data
            /w14263 # 'function': member function does not override any base class virtual member function
            /w14265 # 'classname': class has virtual functions, but destructor is not virtual instances of this class may not
            # be destructed correctly
            /w14287 # 'operator': unsigned/negative constant mismatch
            /we4289 # nonstandard extension used: 'variable': loop control variable declared in the for-loop is used outside
            # the for-loop scope
            /w14296 # 'operator': expression is always 'boolean_value'
            /w14311 # 'variable': pointer truncation from 'type1' to 'type2'
            /w14545 # expression before comma evaluates to a function which is missing an argument list
            /w14546 # function call before comma missing argument list
            /w14547 # 'operator': operator before comma has no effect; expected operator with side-effect
            /w14549 # 'operator': operator before comma has no effect; did you intend 'operator'?
            /w14555 # expression has no effect; expected expression with side- effect
            /w14619 # pragma warning: there is no warning number 'number'
            /w14640 # Enable warning on thread un-safe static member initialization
            /w14826 # Conversion from 'type1' to 'type_2' is sign-extended. This may cause unexpected runtime behavior.
            /w14905 # wide string literal cast to 'LPSTR'
            /w14906 # string literal cast to 'LPWSTR'
            /w14928 # illegal copy-initialization; more than one user-defined conversion has been implicitly applied
            /permissive- # standards conformance mode for MSVC compiler.
            )
    set(MSVC_SAN_OPTIONS
            /fsanitize=address
            /fsanitize=leak
            )

    set(CLANG_GENERAL_DBG
            -g
            -O0
            -fno-inline-functions
            -fdiagnostics-color
            -U_FORTIFY_SOURCE
            -D_FORTIFY_SOURCE=0
            )
    set(CLANG_WARNINGS
            -Wall
            -Wextra # reasonable and standard
            -Wshadow # warn the user if a variable declaration shadows one from a parent context
            -Wnon-virtual-dtor # warn the user if a class with virtual functions has a non-virtual destructor. This helps
            # catch hard to track down memory errors
            -Wold-style-cast # warn for c-style casts
            -Wcast-align # warn for potential performance problem casts
            -Wunused # warn on anything being unused
            -Woverloaded-virtual # warn if you overload (not override) a virtual function
            -Wpedantic # warn if non-standard C++ is used
#            -Wconversion # warn on type conversions that may lose data
#            -Wsign-conversion # warn on sign conversions
            -Wnull-dereference # warn if a null dereference is detected
            -Wdouble-promotion # warn if float is implicit promoted to double
            -Wformat=2 # warn on security issues around functions that format output (ie printf)
            -Werror # warnings are treated as errors
            )
    set(CLANG_SAN_OPTIONS
            -fsanitize=address
            -fsanitize=leak
            -fno-omit-frame-pointer
            )
    set(CLANG_S_TEMPS
            -masm=intel
            -save-temps
            )

    set(GCC_GENERAL_DBG
            -g
            -O0
            -fno-inline
            -fdiagnostics-color
            -U_FORTIFY_SOURCE
            -D_FORTIFY_SOURCE=0
            )
    set(GCC_WARNINGS
            ${CLANG_WARNINGS}
            -Wmisleading-indentation # warn if indentation implies blocks where blocks do not exist
            -Wduplicated-cond # warn if if / else chain has duplicated conditions
            -Wduplicated-branches # warn if if / else branches have duplicated code
            -Wlogical-op # warn about logical operations being used where bitwise were probably wanted
            -Wuseless-cast # warn if you perform a cast to the same type
            )
    set(GCC_SAN_OPTIONS ${CLANG_SAN_OPTIONS})
    set(GCC_S_TEMPS ${CLANG_S_TEMPS})

    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set(GENERAL_DBG ${MSVC_GENERAL_DBG})
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(GENERAL_DBG ${CLANG_GENERAL_DBG})
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(GENERAL_DBG ${GCC_GENERAL_DBG})
    endif()

    if (tar_options_WARNINGS_AS_ERRORS)
        if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
            set(PROJECT_WARNINGS ${MSVC_WARNINGS})
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            set(PROJECT_WARNINGS ${CLANG_WARNINGS})
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            set(PROJECT_WARNINGS ${GCC_WARNINGS})
        endif()
    endif()

    if (tar_options_SANITIZERS)
        if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
            set(SAN_OPTIONS ${MSVC_SAN_OPTIONS})
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            set(SAN_OPTIONS ${CLANG_SAN_OPTIONS})
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            set(SAN_OPTIONS ${GCC_SAN_OPTIONS})
        endif()
    endif()

    if (tar_options_SAVE_TEMP_FILES)
        if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")

        elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            set(S_TEMPS ${CLANG_S_TEMPS})
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            set(S_TEMPS ${GCC_S_TEMPS})
        endif()
    endif()



    target_compile_options(
            ${tar_options_TARGET}
            PRIVATE ${PROJECT_WARNINGS}
            PRIVATE ${SAN_OPTIONS}
            PRIVATE ${S_TEMPS}
            PRIVATE ${GENERAL_DBG}
    )

endfunction(add_debug_compiler_options)