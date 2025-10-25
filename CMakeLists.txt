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

include(FetchContent)
# Use a fork of soxr that has been patched for working with C++ as a library (for a Python wrapper)
FetchContent_Declare(soxr GIT_REPOSITORY https://github.com/dofuuz/soxr.git GIT_TAG a66f3ee)
FetchContent_MakeAvailable(soxr)
target_link_libraries(soxrpp INTERFACE soxr)
target_include_directories(soxrpp
    INTERFACE
        # The soxr headers aren't correctly forwarded for some reason...
        $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
        # "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include ${soxr_SOURCE_DIR}/src>"
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# See https://softwarecraft.ch/cmake-interface-lib/
install(
    TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_LIBDIR})

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