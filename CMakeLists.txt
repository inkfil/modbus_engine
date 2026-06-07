cmake_minimum_required(VERSION 3.29)

# Project name and language.
project(modbus_engine VERSION 1.2 LANGUAGES CXX)

# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/Compiled")

# Access the project name.
message(STATUS "System:                                [${CMAKE_SYSTEM_NAME}]")
message(STATUS "Generator:                             [${CMAKE_GENERATOR}]")
message(STATUS "Shared libs supported:                 [${CMAKE_SHARED_LIBRARY_SUPPORTED}]")
message(STATUS "CMake Build Type:                      [${CMAKE_BUILD_TYPE}]")
message(STATUS "CMake Source Directory is:             [${CMAKE_SOURCE_DIR}]")
message(STATUS "CMake Current Source Directory is:     [${CMAKE_CURRENT_SOURCE_DIR}]")
message(STATUS "CMake Build Directory is:              [${CMAKE_CURRENT_BINARY_DIR}]")
message(STATUS "Project Source Directory is:           [${PROJECT_SOURCE_DIR}]")
message(STATUS "The current project name is:           [${PROJECT_NAME}]") 
message(STATUS "The top-level project name is:         [${CMAKE_PROJECT_NAME}]")
message(STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY:        [${CMAKE_RUNTIME_OUTPUT_DIRECTORY}]")

# print_env_path_as_list(PATH)

# Set C++ standard.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Module path
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_MODULE_PATH:        [${CMAKE_MODULE_PATH}]")

# Include directories
# set(BUILD_SHARED_LIBS OFF)


# Find libmodbus
# Manually search in Homebrew's common paths
set(LIBMODBUS_ROOT "${CMAKE_SOURCE_DIR}/libs/macos/libmodbus-3.1.12/src")
find_path(MODBUS_INCLUDE_DIR NAMES modbus.h
          PATHS /opt/homebrew/include /opt/homebrew/include/modbus /usr/local/include/modbus ${LIBMODBUS_ROOT})

find_library(MODBUS_LIBRARY NAMES modbus
             PATHS /opt/homebrew/lib /usr/local/lib ${LIBMODBUS_ROOT})

 if(NOT MODBUS_INCLUDE_DIR OR NOT MODBUS_LIBRARY)
     message(FATAL_ERROR "libmodbus not found")
 endif()

 message(INFO "Modbus include Dirs: " ${MODBUS_INCLUDE_DIR})

# add_executable(my_app main.c)
# target_include_directories(my_app PRIVATE ${MODBUS_INCLUDE_DIR})
# target_link_libraries(my_app PRIVATE ${MODBUS_LIBRARY})

message(STATUS "Pointer size: ${CMAKE_SIZEOF_VOID_P}")

string(TIMESTAMP BUILD_TIME "%Y-%m-%d %H:%M:%S")
execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Configure a header file to pass version info to C++.
configure_file(
    "${PROJECT_SOURCE_DIR}/include/version.h.in"
    "${PROJECT_SOURCE_DIR}/include/version.h"
    @ONLY
)

set(src_files

    ${PROJECT_SOURCE_DIR}/main.cpp
    ${PROJECT_SOURCE_DIR}/include/version.h
)

message(STATUS "List [Source Files]: ${src_files}")

# Add executable.
add_executable(${PROJECT_NAME} ${src_files})

# enable_linux_warnings(${PROJECT_NAME})

# Add include directories.
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/include
    ${FFMPEG_ROOT}/include
    "/opt/homebrew/include/"
    ${MODBUS_INCLUDE_DIR}
)
target_link_libraries(${PROJECT_NAME} PRIVATE ${MODBUS_LIBRARY})

# Install the executable and its runtime dependencies automatically
# include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME} ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    RUNTIME DEPENDENCIES # Automatically finds and installs associated DLLs/SOs
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

include(InstallRequiredSystemLibraries)

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VENDOR "ronaktheshinigami@gmail.com")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})

# Output file name
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")

# Generator options (choose one or more)
set(CPACK_GENERATOR "ZIP;NSIS")

# NSIS-specific (installer)
set(CPACK_NSIS_DISPLAY_NAME "${PROJECT_NAME}")
set(CPACK_NSIS_PACKAGE_NAME "${PROJECT_NAME} Installer")
set(CPACK_NSIS_EXECUTABLES_DIRECTORY ".")

include(CPack)
