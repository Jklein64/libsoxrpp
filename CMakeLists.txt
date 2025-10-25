cmake_minimum_required(VERSION 3.24)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(soxrpp 
    VERSION 0.0.1 
    DESCRIPTION "C++ wrapper for libsoxr."
    LANGUAGES CXX C)

add_library(soxrpp INTERFACE)
add_library(soxrpp::soxrpp ALIAS soxrpp)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
# Use at least C++20 for span and class-valued non-type templates
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_20)
target_include_directories(soxrpp
    INTERFACE
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
        "$<BUILD_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

include(FetchContent)
# Use a fork of soxr that has been patched for working with C++ as a library (for a Python wrapper)
FetchContent_Declare(soxr GIT_REPOSITORY https://github.com/dofuuz/soxr.git GIT_TAG a66f3ee)
FetchContent_MakeAvailable(soxr)
target_link_libraries(soxrpp INTERFACE soxr)
# Ugly hack because soxr CMake gets confused by modern CMake
# TODO make this private once the wrapper is properly abstracted
# target_include_directories(soxrpp INTERFACE
#     # Can remove generators once PUBLIC becomes PRIVATE
#     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/soxrpp ${soxr_SOURCE_DIR}/src>  
#     $<INSTALL_INTERFACE:include/soxrpp>  # TODO do I need find_package here for soxr?
# )

include(cmake/utils.cmake)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(NOT CMAKE_SKIP_INSTALL_RULES)
    # See https://softwarecraft.ch/cmake-interface-lib/
    install(
        TARGETS ${PROJECT_NAME} soxr
        EXPORT ${PROJECT_NAME}-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    write_basic_package_version_file(
        "${PROJECT_NAME}-config-version.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

    configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
        INSTALL_DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)
    
    install(
        EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}-targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake
        # soxr exports LINK_INTERFACE_LIRARIES, so this is needed for compatibility
        EXPORT_LINK_INTERFACE_LIBRARIES)

    install(
        FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
              "${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)
    install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/ DESTINATION include)
endif()