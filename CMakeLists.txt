#------------------------------------------------------------------------------
# Top-level CMake file for bcos-framework
# ------------------------------------------------------------------------------
# Copyright (C) 2021 FISCO BCOS.
# SPDX-License-Identifier: Apache-2.0
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.12)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")

# cmake settings
set(BCOS_CMAKE_SCRIPTS "bcos-cmake-scripts")
set(BCOS_CMAKE_SCRIPTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${BCOS_CMAKE_SCRIPTS}" CACHE PATH "The path for the ${BCOS_CMAKE_SCRIPTS} subtree")
set(BCOS_FRAMEWORK_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake" CACHE PATH "The cmake path for the library")
list(APPEND CMAKE_MODULE_PATH ${BCOS_CMAKE_SCRIPTS_DIR})
list(APPEND CMAKE_MODULE_PATH ${BCOS_FRAMEWORK_CMAKE_DIR})

# init hunter
set(HUNTER_PACKAGE_LOG_BUILD OFF CACHE STRING "Hunter output build info to log")
# set(HUNTER_STATUS_DEBUG ON CACHE STRING "Hunter debug output")
set(HUNTER_CONFIGURATION_TYPES Release CACHE STRING "Build type of Hunter packages")
include(HunterGate)
HunterGate(
    URL "https://github.com/FISCO-BCOS/hunter/archive/8b5e8f5e01940f175ab3488938119f4a253ec915.tar.gz"
    SHA1 "2cc356093d3c6411304d9c3632241fb457e22ad6"
)

# set CMP0074 to NEW to suppress warnning, https://cmake.org/cmake/help/latest/policy/CMP0074.html
cmake_policy(VERSION 3.12)
cmake_policy(SET CMP0074 NEW)

project(bcos-framework VERSION "3.0.0")
# Suffix like "-rc1" e.t.c. to append to versions wherever needed.
set(VERSION_SUFFIX "")

# basic setting
include(Options)
configure_project()
include(CompilerSettings)

# install dependencies
# install boost dependency
hunter_add_package(Boost COMPONENTS all thread system filesystem chrono iostreams log)
# install protobuf dependency
hunter_add_package(Protobuf)
# install gsl dependency
hunter_add_package(Microsoft.GSL)
find_package(Microsoft.GSL CONFIG REQUIRED)
# install tbb dependency
hunter_add_package(tbb)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_INSTALL_INCLUDEDIR})
set(INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)

# set library target name
set(UTILITIES_TARGET "utilities")
set(CODEC_TARGET "codec")
set(PROTOCOL_TARGET "protocol")
set(PBPROTOCOL_TARGET "protocol-pb")
# define the generated proto file path
set(PROTO_GENERATE_DIR ${CMAKE_SOURCE_DIR}/libprotocol/bcos-proto)

# add subdirectories to build the libraries
add_subdirectory(libutilities)
add_subdirectory(libcodec)
add_subdirectory(libprotocol)
add_subdirectory(libprotocol/protobuf)

# test releated
if (TESTS)
    enable_testing()
    set(CTEST_OUTPUT_ON_FAILURE TRUE)
    add_subdirectory(test)
endif()

include(InstallConfig)
install(
    TARGETS ${UTILITIES_TARGET}
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
    TARGETS ${CODEC_TARGET}
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
    TARGETS ${PROTOCOL_TARGET}
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
    TARGETS ${PBPROTOCOL_TARGET}
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

include(InstallInclude)
# for doxygen
include(BuildDocs)

# for code coverage
if (COVERAGE)
    include(Coverage)
    config_coverage("'/usr*' '${CMAKE_SOURCE_DIR}/bcos-cmake-scripts*' '${CMAKE_SOURCE_DIR}/test/bcos-test*'")
endif ()
