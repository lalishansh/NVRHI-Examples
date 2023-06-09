CMAKE_MINIMUM_REQUIRED(VERSION 3.21)

# START: FIND VCPKG TOOLCHAIN FILE
SET(NO_VCPKG_TOOLCHAIN_FILE TRUE)
FOREACH (THE_TOOLCHAIN_FILE ${CMAKE_TOOLCHAIN_FILE} ${CMAKE_TOOLCHAIN_FILE_OVERRIDE})
    IF (${THE_TOOLCHAIN_FILE} MATCHES "vcpkg.cmake" AND EXISTS ${THE_TOOLCHAIN_FILE})
        SET(NO_VCPKG_TOOLCHAIN_FILE FALSE)
    ENDIF ()
    IF (NOT NO_VCPKG_TOOLCHAIN_FILE)
        break () # there is a vcpkg toolchain file
    ENDIF ()
ENDFOREACH ()
IF (${NO_VCPKG_TOOLCHAIN_FILE})
    IF (DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        LIST(APPEND CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
    ELSEIF (DEFINED ENV{VCPKG_DIR} AND EXISTS "$ENV{VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake")
        LIST(APPEND CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_DIR}/scripts/buildsystems/vcpkg.cmake")
    ELSE ()
        MESSAGE(FATAL_ERROR "No VcPkg toolchain file found")
    ENDIF ()
ENDIF ()
## END: FIND VCPKG TOOLCHAIN FILE

# Read the vcpkg.json file
FILE(READ "${CMAKE_SOURCE_DIR}/vcpkg.json" VCPKG_JSON_RAW)
# Parse "version-string" from JSON content
STRING(JSON VCPKG_JSON_VERSION_STRING GET ${VCPKG_JSON_RAW} "version-string")

# Project information
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
PROJECT(NVRHI_EXAMPLES VERSION ${VCPKG_JSON_VERSION_STRING} LANGUAGES CXX)
SET(THIS_EXEC hello_world)

# Generate version.h file
SET(GENERATED_SOURCE_FILES_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
FILE(MAKE_DIRECTORY ${GENERATED_SOURCE_FILES_DIR})
FILE(WRITE "${GENERATED_SOURCE_FILES_DIR}/version.h"
        "#pragma once\n"
        "// This file is generated by CMake\n"
        "// Do not edit this file\n"
        "#define ${PROJECT_NAME}_VERSION_STRING \"${VCPKG_JSON_VERSION_STRING}\"\n"
        "#define ${PROJECT_NAME}_VERSION_MAJOR ${${PROJECT_NAME}_VERSION_MAJOR}\n"
        "#define ${PROJECT_NAME}_VERSION_MINOR ${${PROJECT_NAME}_VERSION_MINOR}\n"
        "#define ${PROJECT_NAME}_VERSION_PATCH ${${PROJECT_NAME}_VERSION_PATCH}\n"
        )

ADD_SUBDIRECTORY(examples)

# Packaging information
INCLUDE(InstallRequiredSystemLibraries)
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
SET(CPACK_SET_DESTDIR "${CMAKE_CURRENT_SOURCE_DIR}/installers/")
SET(CPACK_PACKAGE_VERSION_MAJOR "${${PROJECT_NAME}_VERSION_MAJOR}")
SET(CPACK_PACKAGE_VERSION_MINOR "${${PROJECT_NAME}_VERSION_MINOR}")
SET(CPACK_PACKAGE_VERSION_PATCH "${${PROJECT_NAME}_VERSION_PATCH}")
SET(CPACK_PACKAGE_CONTACT "Ishansh Lal <t.RED.0_0@proton.me>")

# For creating installers for Platforms
IF (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    SET(CPACK_GENERATOR "NSIS")
ELSEIF (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "Ishansh Lal <t.RED.0_0@proton.me>")
    SET(CPACK_GENERATOR "DEB")
ENDIF ()
INCLUDE(CPack)
