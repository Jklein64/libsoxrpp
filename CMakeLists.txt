cmake_minimum_required(VERSION 3.24)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(soxrpp 
    VERSION 0.0.1 
    DESCRIPTION "C++ wrapper for libsoxr."
    LANGUAGES CXX C)

add_library(soxrpp)
add_library(soxrpp::soxrpp ALIAS soxrpp)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

include(cmake/utils.cmake)
include(GNUInstallDirs)

# SOXRPP_SHARED_LIBS determines static/shared build when defined
option(SOXRPP_INSTALL "Generate target for installing soxrpp" ${PROJECT_IS_TOP_LEVEL})
set_if_undefined(SOXRPP_INSTALL_CMAKEDIR 
    "${CMAKE_INSTALL_LIBDIR}/cmake/soxrpp-${PROJECT_VERSION}" CACHE STRING 
    "Install path for soxrpp package-related CMake files")

if(DEFINED SOXRPP_SHARED_LIBS)
    set(BUILD_SHARED_LIBS ${SOXRPP_SHARED_LIBS})
endif()

set_if_undefined(CMAKE_CXX_VISIBILITY_PRESET hidden)
set_if_undefined(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

include(GenerateExportHeader)
set(export_file_name "export_shared.h")

if(NOT BUILD_SHARED_LIBS)
    set(export_file_name "export_static.h")
endif()

generate_export_header(soxrpp EXPORT_FILE_NAME
    include/soxrpp/${export_file_name})

set(public_headers
    include/soxrpp/export.h
    include/soxrpp/soxrpp.h)
set(sources
    ${public_headers}
    src/soxrpp.cpp)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

list(APPEND public_headers
    "${CMAKE_CURRENT_BINARY_DIR}/include/soxrpp/${export_file_name}")
list(APPEND sources
    "${CMAKE_CURRENT_BINARY_DIR}/include/soxrpp/${export_file_name}")

include(CMakePackageConfigHelpers)

target_sources(soxrpp PRIVATE ${sources})
target_compile_definitions(soxrpp PUBLIC "$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:SOXRPP_STATIC_DEFINE>")

target_include_directories(soxrpp
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>")

set_target_properties(soxrpp PROPERTIES
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION})

include(FetchContent)
# Use a fork of soxr that has been patched for working with C++ as a library (for a Python wrapper)
FetchContent_Declare(soxr GIT_REPOSITORY https://github.com/dofuuz/soxr.git GIT_TAG a66f3ee)
FetchContent_MakeAvailable(soxr)
target_link_libraries(soxrpp PRIVATE soxr)
# Ugly hack because soxr CMake gets confused by modern CMake
target_include_directories(soxrpp PRIVATE ${soxr_SOURCE_DIR}/src)

if(SOXRPP_INSTALL AND NOT CMAKE_SKIP_INSTALL_RULES)
    configure_package_config_file(cmake/soxrpp-config.cmake.in soxrpp-config.cmake
        INSTALL_DESTINATION "${SOXRPP_INSTALL_CMAKEDIR}")

    write_basic_package_version_file(soxrpp-config-version.cmake
        COMPATIBILITY SameMajorVersion)

    install(TARGETS soxrpp soxr EXPORT soxrpp_export
        RUNTIME COMPONENT soxrpp
        LIBRARY COMPONENT soxrpp NAMELINK_COMPONENT soxrpp-dev
        ARCHIVE COMPONENT soxrpp-dev
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
    install(DIRECTORY include/
        TYPE INCLUDE
        COMPONENT soxrpp-dev)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/soxrpp/${export_file_name}"
        COMPONENT soxrpp-dev
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/soxrpp")

    set(targets_file "soxrpp-shared-targets.cmake")

    if(NOT BUILD_SHARED_LIBS)
        set(targets_file "soxrpp-static-targets.cmake")
    endif()

    install(EXPORT soxrpp_export
        COMPONENT soxrpp-dev
        FILE "${targets_file}"
        DESTINATION "${SOXRPP_INSTALL_CMAKEDIR}"
        EXPORT_LINK_INTERFACE_LIBRARIES
        NAMESPACE soxrpp::)

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/soxrpp-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/soxrpp-config-version.cmake"
        COMPONENT soxrpp-dev
        DESTINATION "${SOXRPP_INSTALL_CMAKEDIR}")
endif()