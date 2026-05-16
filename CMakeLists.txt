cmake_minimum_required(VERSION 3.30)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(CAVeTalk LANGUAGES C CXX)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CAVETALK_TARGET cavetalk)
set(CAVETALK_TARGET_C ${CAVETALK_TARGET}c)
set(CAVETALK_TARGET_CPP ${CAVETALK_TARGET}c++)
set(EXTERNAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external)
set(GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/generated)

option(CAVETALK_BUILD_C "Build CAVeTalk C library" ON)
option(CAVETALK_BUILD_CPP "Build CAVeTalk C++ library" ON)
option(CAVETALK_BUILD_TESTS "Build CAVeTalk tests" OFF)

################################################################################
# Messages sources
################################################################################
set(CAVETALK_MESSAGES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/messages)
file(GLOB CAVETALK_MESSAGE_SRCS LIST_DIRECTORIES false CONFIGURE_DEPENDS
    "${CAVETALK_MESSAGES_DIR}/*.proto"
)

################################################################################
# C messages
################################################################################
if (CAVETALK_BUILD_C)
    set(C_MESSAGES_OUT_DIR ${GENERATED_DIR}/${CAVETALK_TARGET_C}_protos)
    file(GLOB CAVETALK_C_MESSAGE_SRCS LIST_DIRECTORIES false CONFIGURE_DEPENDS
        "${C_MESSAGES_OUT_DIR}/*.pb.c"
    )
    add_library(${CAVETALK_TARGET_C}_messages)
    target_sources(${CAVETALK_TARGET_C}_messages
        PRIVATE
            ${CAVETALK_C_MESSAGE_SRCS}
            ${C_MESSAGES_OUT_DIR}/pb_common.c
            ${C_MESSAGES_OUT_DIR}/pb_decode.c
            ${C_MESSAGES_OUT_DIR}/pb_encode.c
    )
    target_include_directories(${CAVETALK_TARGET_C}_messages
        PUBLIC
            ${C_MESSAGES_OUT_DIR}
    )
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${CAVETALK_TARGET_C}_messages
            PRIVATE
                -fPIC
        )
    # Add flags for other compilers here
    endif()
endif()

################################################################################
# C library
################################################################################
set(C_DIR ${CMAKE_CURRENT_SOURCE_DIR}/c)
set(C_INC_DIR ${C_DIR}/include)
set(C_SRC_DIR ${C_DIR}/src)
set(C_SRCS ${C_SRC_DIR}/cavetalk.c)
if (CAVETALK_BUILD_C)
    add_library(${CAVETALK_TARGET_C})
    add_library(${CAVETALK_TARGET}::${CAVETALK_TARGET_C} ALIAS ${CAVETALK_TARGET_C})
    target_sources(${CAVETALK_TARGET_C}
        PRIVATE
            ${C_SRCS}
    )
    target_include_directories(${CAVETALK_TARGET_C}
        PUBLIC
            ${C_INC_DIR}
    )
    target_link_libraries(${CAVETALK_TARGET_C}
        PUBLIC
            ${CAVETALK_TARGET_C}_messages
    )
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${CAVETALK_TARGET_C}
            PRIVATE
                -Wall -Wextra -Werror -Wpedantic -fdiagnostics-color=always
                -fPIC
                $<$<CONFIG:Debug>:-g -O0 --coverage>
        )
        target_link_options(${CAVETALK_TARGET_C}
            PRIVATE
                $<$<CONFIG:Debug>:--coverage>
        )
    # Add flags for other compilers here
    endif()
endif()

################################################################################
# C++ messages
################################################################################
if (CAVETALK_BUILD_CPP)
    option(ABSL_PROPAGATE_CXX_STD "Propagate C++ standard to abseil-cpp" ON)
    set(PROTOBUF_INSTALL_DIR ${EXTERNAL_DIR}/protobuf/_build/protobuf-install)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/protobuf/third_party/abseil-cpp)
    find_package(utf8_range CONFIG REQUIRED HINTS ${PROTOBUF_INSTALL_DIR})
    find_package(Protobuf CONFIG REQUIRED HINTS ${PROTOBUF_INSTALL_DIR})
    set(CPP_MESSAGES_OUT_DIR ${GENERATED_DIR}/${CAVETALK_TARGET_CPP}_protos)
    add_library(${CAVETALK_TARGET_CPP}_messages)
    protobuf_generate(
        TARGET ${CAVETALK_TARGET_CPP}_messages
        LANGUAGE cpp
        PROTOS ${CAVETALK_MESSAGE_SRCS}
        IMPORT_DIRS ${CAVETALK_MESSAGES_DIR}
        PROTOC_OUT_DIR ${CPP_MESSAGES_OUT_DIR}
    )
    target_link_libraries(${CAVETALK_TARGET_CPP}_messages PRIVATE protobuf::libprotobuf)
    target_include_directories(${CAVETALK_TARGET_CPP}_messages
        PUBLIC
            ${PROTOBUF_INSTALL_DIR}/include
            ${CPP_MESSAGES_OUT_DIR}
    )
    target_compile_options(${CAVETALK_TARGET_CPP}_messages PRIVATE -Wno-missing-requires)
endif()

################################################################################
# C++ library
################################################################################
set(CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/c++)
set(CPP_INC_DIR ${CPP_DIR}/include)
set(CPP_SRC_DIR ${CPP_DIR}/src)
set(CPP_SRCS ${CPP_SRC_DIR}/cavetalk.cc)
if (CAVETALK_BUILD_CPP)
    add_library(${CAVETALK_TARGET_CPP})
    add_library(${CAVETALK_TARGET}::${CAVETALK_TARGET_CPP} ALIAS ${CAVETALK_TARGET_CPP})
    target_sources(${CAVETALK_TARGET_CPP}
        PRIVATE
            ${CPP_SRCS}
    )
    target_include_directories(${CAVETALK_TARGET_CPP}
        PUBLIC
            ${CPP_INC_DIR}
    )
    target_link_libraries(${CAVETALK_TARGET_CPP}
        PUBLIC
            ${CAVETALK_TARGET_CPP}_messages
    )
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${CAVETALK_TARGET_CPP}
            PRIVATE
                -Wall -Wextra -Werror -Wno-missing-requires -fdiagnostics-color=always
                -fPIC
                $<$<CONFIG:Debug>:-g -O0 --coverage>
        )
        target_link_options(${CAVETALK_TARGET_CPP}
            PRIVATE
                $<$<CONFIG:Debug>:--coverage>
        )
    # Add flags for other compilers here
    endif()
endif()

################################################################################
# CI and tools
################################################################################
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(TOOLS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tools)
    set(CPPCHECK_SOURCES ${CPP_SRCS} ${C_SRCS})
    set(UNCRUSTIFY_INC_DIRS ${CPP_INC_DIR} ${C_INC_DIR})
    if (CPPCHECK_SOURCES)
        include(${TOOLS_DIR}/cppcheck/cppcheck.cmake)
    endif()
    if (CPPCHECK_SOURCES AND UNCRUSTIFY_INC_DIRS)
        # TODO add tests to uncrustify sources
        set(UNCRUSTIFY_SOURCES ${CPPCHECK_SOURCES})
        include(${TOOLS_DIR}/uncrustify/uncrustify.cmake)
    endif()
endif()

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
        string(REPLACE "+" "\\\\\\+" GCOVR_C_DIR "${C_DIR}")
        string(REPLACE "+" "\\\\\\+" GCOVR_CPP_DIR "${CPP_DIR}")
        set(GCOVR_ARGS
            -r ${CMAKE_CURRENT_SOURCE_DIR}
            -f ${GCOVR_C_DIR}
            -f ${GCOVR_CPP_DIR}
            --exclude-throw-branches
            --gcov-ignore-parse-errors=suspicious_hits.warn
            --gcov-ignore-parse-errors=negative_hits.warn
        )
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
