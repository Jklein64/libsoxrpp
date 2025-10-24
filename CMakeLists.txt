cmake_minimum_required(VERSION 3.24)

project(libsoxrpp 
    VERSION 0.0.1 
    DESCRIPTION "C++ wrapper for libsoxr."
    LANGUAGES CXX)

add_library(libsoxrpp)
add_library(libsoxrpp::libsoxrpp ALIAS libsoxrpp)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

include(cmake/utils.cmake)
include(GNUInstallDirs)

# LIBSOXRPP_SHARED_LIBS determines static/shared build when defined
option(LIBSOXRPP_INSTALL "Generate target for installing libsoxrpp" ${PROJECT_IS_TOP_LEVEL})
set_if_undefined(LIBSOXRPP_INSTALL_CMAKEDIR 
    "${CMAKE_INSTALL_LIBDIR}/cmake/libsoxrpp-${PROJECT_VERSION}" CACHE STRING 
    "Install path for libsoxrpp package-related CMake files")

if(DEFINED LIBSOXRPP_SHARED_LIBS)
    set(BUILD_SHARED_LIBS ${LIBSOXRPP_SHARED_LIBS})
endif()

set_if_undefined(CMAKE_CXX_VISIBILITY_PRESET hidden)
set_if_undefined(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

include(GenerateExportHeader)
set(export_file_name "export_shared.h")

if(NOT BUILD_SHARED_LIBS)
    set(export_file_name "export_static.h")
endif()

generate_export_header(libsoxrpp EXPORT_FILE_NAME
    include/libsoxrpp/${export_file_name})

set(public_headers
    include/libsoxrpp/export.h
    include/libsoxrpp/libsoxrpp.h)
set(sources
    ${public_headers}
    src/libsoxrpp.cpp)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${sources})

list(APPEND public_headers
    "${CMAKE_CURRENT_BINARY_DIR}/include/libsoxrpp/${export_file_name}")
list(APPEND sources
    "${CMAKE_CURRENT_BINARY_DIR}/include/libsoxrpp/${export_file_name}")

include(CMakePackageConfigHelpers)

target_sources(libsoxrpp PRIVATE ${sources})
target_compile_definitions(libsoxrpp PUBLIC "$<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:LIBSOXRPP_STATIC_DEFINE>")

target_include_directories(libsoxrpp
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>")

set_target_properties(libsoxrpp PROPERTIES
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION})

if(LIBSOXRPP_INSTALL AND NOT CMAKE_SKIP_INSTALL_RULES)
    configure_package_config_file(cmake/libsoxrpp-config.cmake libsoxrpp-config.cmake
        INSTALL_DESTINATION "${LIBSOXRPP_INSTALL_CMAKEDIR}")

    write_basic_package_version_file(libsoxrpp-config-version.cmake
        COMPATIBILITY SameMajorVersion)

    install(TARGETS libsoxrpp EXPORT libsoxrpp_export
        RUNTIME COMPONENT libsoxrpp
        LIBRARY COMPONENT libsoxrpp NAMELINK_COMPONENT libsoxrpp-dev
        ARCHIVE COMPONENT libsoxrpp-dev
        INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
    install(DIRECTORY include/
        TYPE INCLUDE
        COMPONENT libsoxrpp-dev)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/include/libsoxrpp/${export_file_name}"
        COMPONENT libsoxrpp-dev
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/libsoxrpp")

    set(targets_file "libsoxrpp-shared-targets.cmake")

    if(NOT BUILD_SHARED_LIBS)
        set(targets_file "libsoxrpp-static-targets.cmake")
    endif()

    install(EXPORT libsoxrpp_export
        COMPONENT libsoxrpp-dev
        FILE "${targets_file}"
        DESTINATION "${LIBSOXRPP_INSTALL_CMAKEDIR}"
        NAMESPACE libsoxrpp::)

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/libsoxrpp-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/libsoxrpp-config-version.cmake"
        COMPONENT libsoxrpp-dev
        DESTINATION "${LIBSOXRPP_INSTALL_CMAKEDIR}")
endif()