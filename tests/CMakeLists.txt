cmake_minimum_required(VERSION 3.30)

string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}" IS_TOP_LEVEL)

if(IS_TOP_LEVEL)
    project(CAVeTalk-Tests LANGUAGES C CXX)
    enable_testing()
endif()

set(CMAKE_CXX_STANDARD 20)
set(CAVETALK_TEST_TARGET cavetalk-tests)

################################################################################
# GoogleTest
################################################################################
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/6910c9d9165801d8827d628cb72eb7ea9dd538c5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

################################################################################
# CAVeTalk library
################################################################################
if(IS_TOP_LEVEL)
    find_package(cavetalkc REQUIRED)
endif()

################################################################################
# CAVeTalk tests
################################################################################
set(${CAVETALK_TEST_TARGET}_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cavetalkc_tests.cc
)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${${CAVETALK_TEST_TARGET}_SRCS})
add_executable(${CAVETALK_TEST_TARGET})
target_sources(${CAVETALK_TEST_TARGET}
    PRIVATE
        ${${CAVETALK_TEST_TARGET}_SRCS}
)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${CAVETALK_TEST_TARGET}
        PRIVATE
            -fdiagnostics-color=always
    )
    target_link_options(${CAVETALK_TEST_TARGET}
        PRIVATE
            $<$<CONFIG:Debug>:--coverage>
    )
# Add flags for other compilers here
endif()
target_link_libraries(${CAVETALK_TEST_TARGET}
    PUBLIC
        cavetalk::cavetalkc
        GTest::gtest_main
        GTest::gmock
)
gtest_discover_tests(${CAVETALK_TEST_TARGET})
