cmake_minimum_required(VERSION 3.26)
project(jsonlib VERSION 0.1.0 LANGUAGES CXX)

option(JSONLIB_BUILD_TESTS "Build unit tests" ON)
option(JSONLIB_USE_PMR "Enable PMR allocators" ON)
option(JSONLIB_USE_EXCEPTIONS "Enable exception in API" OFF)

# # Compiler settings 
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# for MSVC: use static runtime /MT and debug static runtime (/MTd) 
# in Debug builds.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Enable AddressSanitizer for MSVC in Debug mode:
#   /fsanitize=address : Enable AddressSanitizer (detects memory errors)
#   /Zi                : Generate debug info
#   /INCREMENTAL:NO    : Disable incremental linking (required for ASan)
# set(sanitize_flags "/fsanitize=address /Zi /INCREMENTAL:NO")

# define library target name (jsonlib):
add_library(jsonlib 
    src/json_value.cpp
    src/json_parse.cpp
    src/json_stringify.cpp
)
target_include_directories(jsonlib PUBLIC
    # Tell CMake where headers are:
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>      # for use in build tree
    $<INSTALL_INTERFACE:include>                        # for use in install tree
)
# Define macros depending on options, so code can #ifdef them:
target_compile_definitions(jsonlib PUBLIC
    # If JSONLIB_USE_PMR=ON, add -DJSONLIB_USE_PMR=1 to compiler flags.
    $<$<BOOL:${JSONLIB_USE_PMR}>:JSONLIB_USE_PMR=1>

    # If JSONLIB_USE_EXCEPTIONS=ON, add -DJSONLIB_USE_EXCEPTIONS=1 to compiler flags.  
    $<$<BOOL:${JSONLIB_USE_EXCEPTIONS}>:JSONLIB_USE_EXCEPTIONS=1>
)

if (MSVC)
    # add extra compiler options:
    # - /W4: warning level 4 (high, show more potential issues than default /W3)
    # - /permissive-: force strict standards conformance mode 
    #       (https://learn.microsoft.com/en-us/cpp/build/reference/permissive-standards-conformance?view=msvc-170)
    # - /utf-8: source files are in UTF-8 encoding                  
    target_compile_options(jsonlib PRIVATE /W4 /permissive- /utf-8)

    # ## "$<$<CONFIG:Debug>:...>": only apply these flags if we’re building in Debug mode.
    # target_compile_options(jsonlib PRIVATE "$<$<CONFIG:Debug>:${sanitize_flags}>")
    # target_link_options(jsonlib PRIVATE "$<$<CONFIG:Debug>:${sanitize_flags}>")
endif()

if (JSONLIB_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0)
    FetchContent_MakeAvailable(googletest)  # ensures the named dependencies have been populated
    enable_testing()
    add_executable(jsonlib_tests 
        tests/test_smoke.cpp
        tests/test_parse.cpp
        tests/test_stringify.cpp
    )
    target_link_libraries(jsonlib_tests PRIVATE jsonlib GTest::gtest_main)
    # target_compile_options(jsonlib_tests PRIVATE "$<$<CONFIG:Debug>:${sanitize_flags}>")
    add_test(NAME jsonlib_tests COMMAND jsonlib_tests)
endif()
