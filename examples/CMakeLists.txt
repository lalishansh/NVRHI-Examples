CMAKE_MINIMUM_REQUIRED(VERSION 3.21)

# Copy the properties from the main target to the test target
FUNCTION(COPY_PROPERTY_FROM_TARGET_TO_TARGET target_from target_to property_name)
    GET_PROPERTY(the_property TARGET ${target_from} PROPERTY ${property_name})
    IF (NOT the_property STREQUAL "the_property-NOTFOUND")
        SET_PROPERTY(TARGET ${target_to} PROPERTY ${property_name} ${the_property})
    ENDIF ()
ENDFUNCTION()

# add test target for target
# param target - target name
# param test_target - test target name
# param ARGN - list of properties to copy from target to test_target
FUNCTION(ADD_TESTS_TARGET_FOR_TARGET target test_target)
    # get the source-dir of the target
    GET_TARGET_PROPERTY(target_src_dir ${target} EXAMPLE_SOURCE_DIR)
    IF (target_src_dir STREQUAL "target_src_dir-NOTFOUND")
        MESSAGE(WARNING "Target ${target} does not have EXAMPLE_SOURCE_DIR property set, will not build tests for it")
        RETURN()
    ENDIF ()

    MESSAGE(STATUS "Building test target for ${target}")

    # collect <source-dir>/tests/**/*.cpp files
    FILE(GLOB_RECURSE test_src_files
            "${target_src_dir}/tests/**.c"
            "${target_src_dir}/tests/**.cc"
            "${target_src_dir}/tests/**.cpp"
            "${target_src_dir}/tests/**.cxx"
            "${target_src_dir}/tests/**.cppm"
            )

    # add test target
    ADD_EXECUTABLE(${test_target} ${test_src_files})
    IF (NOT TARGET Boost::unit_test_framework)
        FIND_PACKAGE(Boost REQUIRED COMPONENTS unit_test_framework)
    ENDIF ()

    TARGET_LINK_LIBRARIES(${test_target} PRIVATE Boost::unit_test_framework)

    # copy properties from target to test_target
    FOREACH (property_name IN LISTS ARGN)
        COPY_PROPERTY_FROM_TARGET_TO_TARGET(${target} ${test_target} ${property_name})
    ENDFOREACH ()
ENDFUNCTION()

# add example target
# param example_dir - example target directory, same as example name
FUNCTION(BUILD_EXAMPLE example_dir)
    GET_FILENAME_COMPONENT(example_target "${example_dir}" NAME)
    IF (NOT EXISTS ${example_dir})
        MESSAGE(FATAL_ERROR "Example Dir: \"${example_dir}\" does not exist")
    ELSEIF (TARGET ${example_target})
        MESSAGE(WARNING "Example ${example_target} already exists, skipping BUILD_EXAMPLE")
        RETURN()
    ELSEIF (EXISTS ${example_dir}/CMakeLists.txt)
        MESSAGE(WARNING
                "Example ${example_target} has a CMakeLists.txt file, using add_subdirectory instead of BUILD_EXAMPLE")
        ADD_SUBDIRECTORY(${example_dir})
        RETURN()
    ENDIF ()

    MESSAGE(STATUS "Building example ${example_target} in ${example_dir}")
    FILE(GLOB_RECURSE example_src_files
            # header files
            "${example_dir}/src/**.h"
            "${example_dir}/src/**.hh"
            "${example_dir}/src/**.hpp"
            "${example_dir}/src/**.hxx"
            "${example_dir}/src/**.ixx"
            # source files
            "${example_dir}/src/**.c"
            "${example_dir}/src/**.cc"
            "${example_dir}/src/**.cpp"
            "${example_dir}/src/**.cxx"
            "${example_dir}/src/**.cppm"
            )

    ADD_EXECUTABLE(${example_target} ${example_src_files})
    SET_TARGET_PROPERTIES(${example_target}
            PROPERTIES
                EXAMPLE_SOURCE_DIR ${example_dir}
            )

    TARGET_INCLUDE_DIRECTORIES(${example_target} PRIVATE
            ${example_dir}/
            ${example_dir}/src/
            )

    IF (NOT TARGET fmt::fmt)
        FIND_PACKAGE(fmt CONFIG REQUIRED)
    ENDIF ()
    IF (NOT TARGET glfw)
        FIND_PACKAGE(glfw3 CONFIG REQUIRED)
    ENDIF ()
    IF (NOT TARGET glm::glm)
        FIND_PACKAGE(glm CONFIG REQUIRED)
    ENDIF ()

    SET(NVRHI_DIR ${CMAKE_SOURCE_DIR}/third_party/nvrhi)
    IF (NOT TARGET nvrhi)
        ADD_SUBDIRECTORY(${NVRHI_DIR} ${CMAKE_BINARY_DIR}/nvrhi)
    ENDIF ()

    IF (NOT TARGET Vulkan::Vulkan)
        FIND_PACKAGE(Vulkan REQUIRED)
    ENDIF ()

    TARGET_LINK_LIBRARIES(${example_target}
            PRIVATE
            fmt::fmt glfw glm::glm Vulkan::Vulkan
            nvrhi nvrhi_vk
            )
    TARGET_INCLUDE_DIRECTORIES(${example_target}
            PRIVATE
            ${GENERATED_SOURCE_FILES_DIR}
            ${NVRHI_DIR}/include
            ${NVRHI_DIR}/thirdparty
            ${CMAKE_SOURCE_DIR}
            )
    TARGET_COMPILE_OPTIONS(${example_target}
            PRIVATE
            $<$<CXX_COMPILER_ID:MSVC>:/WX /W4>
            $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror -Wconversion>
            )

    # test target
    ADD_TESTS_TARGET_FOR_TARGET(${example_target} ${example_target}_tests
            INCLUDE_DIRECTORIES
            COMPILE_DEFINITIONS
            COMPILE_OPTIONS
            LINK_LIBRARIES
            LINK_DIRECTORIES
            SOURCE_DIR
            )

    # Install the example to the install directory
    SET(example_install_dir ./bin)
    INSTALL(TARGETS ${example_target} RUNTIME DESTINATION ${example_install_dir})
    # Install the dependencies to the install directory
    GET_PROPERTY(this_target_linked_libs TARGET ${example_target} PROPERTY LINK_LIBRARIES)
    FOREACH (lib IN LISTS this_target_linked_libs)
        GET_TARGET_PROPERTY(lib_type ${lib} TYPE)
        IF (lib_type STREQUAL "INTERFACE_LIBRARY" OR lib_type STREQUAL "STATIC_LIBRARY")
            CONTINUE()
        ENDIF ()

        IF (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
            GET_TARGET_PROPERTY(lib_location ${lib} LOCATION_DEBUG)
        ELSE ()
            GET_TARGET_PROPERTY(lib_location ${lib} LOCATION_RELEASE)
        ENDIF ()
        IF (lib_location)
            INSTALL(FILES ${lib_location} DESTINATION ${example_install_dir} CONFIGURATIONS ${CMAKE_BUILD_TYPE})
        ENDIF ()
    ENDFOREACH ()
ENDFUNCTION()

# for every example in the examples directory
# add a test target and an example target
FILE(GLOB example_dirs ${CMAKE_CURRENT_SOURCE_DIR}/*)
FOREACH (example_dir IN LISTS example_dirs)
    IF (IS_DIRECTORY ${example_dir})
        BUILD_EXAMPLE(${example_dir})
    ENDIF ()
ENDFOREACH ()
