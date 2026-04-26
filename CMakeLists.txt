cmake_minimum_required(VERSION 3.30)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(CAVeTalk LANGUAGES C CXX)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CAVETALK_TARGET cavetalk)
set(EXTERNAL_DIR ${CMAKE_SOURCE_DIR}/external)

option(CAVETALK_BUILD_TESTS "Build CAVeTalk tests" OFF)

################################################################################
# Messages sources
################################################################################
set(CAVETALK_MESSAGES_DIR ${CMAKE_SOURCE_DIR}/messages)
file(GLOB CAVETALK_MESSAGE_SRCS LIST_DIRECTORIES false CONFIGURE_DEPENDS
    "${CAVETALK_MESSAGES_DIR}/*.proto"
)

################################################################################
# C messages
################################################################################
set(C_MESSAGES_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CAVETALK_TARGET}-c_protos)
file(GLOB CAVETALK_C_MESSAGE_SRCS LIST_DIRECTORIES false CONFIGURE_DEPENDS
    "${C_MESSAGES_OUT_DIR}/*.pb.c"
)
add_library(${CAVETALK_TARGET}-c_messages)
target_sources(${CAVETALK_TARGET}-c_messages
    PRIVATE
        ${CAVETALK_C_MESSAGE_SRCS}
        ${C_MESSAGES_OUT_DIR}/pb_common.c
        ${C_MESSAGES_OUT_DIR}/pb_decode.c
        ${C_MESSAGES_OUT_DIR}/pb_encode.c
)
target_include_directories(${CAVETALK_TARGET}-c_messages
    PUBLIC
        ${C_MESSAGES_OUT_DIR}
)

################################################################################
# C library
################################################################################
set(C_DIR ${CMAKE_CURRENT_SOURCE_DIR}/c)
set(C_INC_DIR ${C_DIR}/include)
set(C_SRC_DIR ${C_DIR}/src)
set(C_SRCS ${C_SRC_DIR}/cavetalk.c)
add_library(${CAVETALK_TARGET}-c)
target_sources(${CAVETALK_TARGET}-c
    PRIVATE
        ${C_SRCS}
)
target_include_directories(${CAVETALK_TARGET}-c
    PUBLIC
        ${C_INC_DIR}
)
target_link_libraries(${CAVETALK_TARGET}-c
    PUBLIC
        ${CAVETALK_TARGET}-c_messages
)
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${CAVETALK_TARGET}-c
        PRIVATE
            -Wall -Wextra -Werror -Wpedantic -fdiagnostics-color=always
            -fPIC
            $<$<CONFIG:Debug>:-g -O0 --coverage>
    )
    target_link_options(${CAVETALK_TARGET}-c
        PRIVATE
            $<$<CONFIG:Debug>:--coverage>
    )
# Add flags for other compilers here
endif()

################################################################################
# C++ messages
################################################################################
option(ABSL_PROPAGATE_CXX_STD "Propagate C++ standard to abseil-cpp" ON)
set(PROTOBUF_INSTALL_DIR ${EXTERNAL_DIR}/protobuf/_build/protobuf-install)
add_subdirectory(${CMAKE_SOURCE_DIR}/external/protobuf/third_party/abseil-cpp)
find_package(utf8_range CONFIG REQUIRED HINTS ${PROTOBUF_INSTALL_DIR})
find_package(Protobuf CONFIG REQUIRED HINTS ${PROTOBUF_INSTALL_DIR})
set(CPP_MESSAGES_OUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CAVETALK_TARGET}-cpp_protos)
add_library(${CAVETALK_TARGET}-cpp_messages)
protobuf_generate(
	TARGET ${CAVETALK_TARGET}-cpp_messages
	LANGUAGE cpp
	PROTOS ${CAVETALK_MESSAGE_SRCS}
	IMPORT_DIRS ${CAVETALK_MESSAGES_DIR}
    PROTOC_OUT_DIR ${CPP_MESSAGES_OUT_DIR}
)
target_link_libraries(${CAVETALK_TARGET}-cpp_messages PRIVATE protobuf::libprotobuf)
target_include_directories(${CAVETALK_TARGET}-cpp_messages
    PUBLIC
        ${PROTOBUF_INSTALL_DIR}/include
        ${CPP_MESSAGES_OUT_DIR}
)
target_compile_options(${CAVETALK_TARGET}-cpp_messages PRIVATE -Wno-missing-requires)

################################################################################
# C++ library
################################################################################
set(CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/c++)
set(CPP_INC_DIR ${CPP_DIR}/inc)
set(CPP_SRC_DIR ${CPP_DIR}/src)
set(CPP_SRCS ${CPP_SRC_DIR}/cavetalk.cc)
add_library(${CAVETALK_TARGET}-cpp)
target_sources(${CAVETALK_TARGET}-cpp
    PRIVATE
        ${CPP_SRCS}
)
target_include_directories(${CAVETALK_TARGET}-cpp
    PUBLIC
        ${CPP_INC_DIR}
)
target_link_libraries(${CAVETALK_TARGET}-cpp
    PUBLIC
        ${CAVETALK_TARGET}-cpp_messages
)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${CAVETALK_TARGET}-cpp
        PRIVATE
            -Wall -Wextra -Werror -Wno-missing-requires -fdiagnostics-color=always
            -fPIC
            $<$<CONFIG:Debug>:-g -O0 --coverage>
    )
    target_link_options(${CAVETALK_TARGET}-cpp
        PRIVATE
            $<$<CONFIG:Debug>:--coverage>
    )
# Add flags for other compilers here
endif()

################################################################################
# CI and tools
################################################################################
set(TOOLS_DIR ${CMAKE_SOURCE_DIR}/tools)
set(CPPCHECK_SOURCES ${COMMON_SRCS} ${CPP_SRCS} ${C_SRCS})
include(${TOOLS_DIR}/cppcheck/cppcheck.cmake)
set(UNCRUSTIFY_SOURCES ${CPPCHECK_SOURCES})
set(UNCRUSTIFY_INC_DIRS ${COMMON_INC_DIR} ${CPP_INC_DIR} ${C_INC_DIR})
include(${TOOLS_DIR}/uncrustify/uncrustify.cmake)

################################################################################
# Tests
################################################################################
if(CAVETALK_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

################################################################################
# Coverage
################################################################################
find_program(GCOVR_PATH gcovr
    HINTS ~/.local/bin/
)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    if(GCOVR_PATH)
        message(STATUS "Found Gcovr at: ${GCOVR_PATH}")
        set(GCOVR_ARGS -r ${CMAKE_BINARY_DIR} -f ${LIB_DIR} --exclude-throw-branches)
        if(DEFINED ENV{GCOV_EXECUTABLE})
            set(GCOVR_ARGS --gcov-executable $ENV{GCOV_EXECUTABLE} ${GCOVR_ARGS})
        endif()
        add_custom_target(
            coverage
            COMMAND ${GCOVR_PATH} ${GCOVR_ARGS} --html-details --html-theme github.dark-green -o coverage.html
            COMMENT "Generating code coverage report..."
            DEPENDS test
        )
        add_custom_target(
            coverage-no-test
            COMMAND ${GCOVR_PATH} ${GCOVR_ARGS} --html-details --html-theme github.dark-green -o coverage.html
            COMMENT "Generating code coverage report..."
        )
        add_custom_target(
            sonarqube-coverage
            COMMAND ${GCOVR_PATH} ${GCOVR_ARGS} --sonarqube -o coverage.xml
            COMMENT "Generating code coverage report..."
        )
    else()
        message(WARNING "Gcovr not found.")
    endif()
else()
    message(WARNING "Gcovr does not support compiler.")
endif()
