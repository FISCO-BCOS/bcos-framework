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

cmake_minimum_required(VERSION 3.10)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version")

# cmake settings
set(BCOS_CMAKE_SCRIPTS "bcos-cmake-scripts")
set(BCOS_CMAKE_SCRIPTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${BCOS_CMAKE_SCRIPTS}" CACHE PATH "The path for the ${BCOS_CMAKE_SCRIPTS} subtree")
set(BCOS_FRAMEWORK_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake" CACHE PATH "The cmake path for the library")
list(APPEND CMAKE_MODULE_PATH ${BCOS_CMAKE_SCRIPTS_DIR})
list(APPEND CMAKE_MODULE_PATH ${BCOS_FRAMEWORK_CMAKE_DIR})

# init hunter
include(HunterGate)
HunterGate(
    URL "https://github.com/cyjseagull/hunter/archive/7b6e6048d4f83314826e19135b39725c5ccdd57e.tar.gz"
    SHA1 "0646a14886cf0a7943e8b0130a72a4c090bfed0e"
)

project(bcos-framework VERSION "3.0")
# Suffix like "-rc1" e.t.c. to append to versions wherever needed.
set(VERSION_SUFFIX "")

# basic setting
include(Options)
configure_project()
include(CompilerSettings)

# install headers firstly
include(CopyInclude)

# install dependencies
# install boost dependency
hunter_add_package(Boost COMPONENTS thread system filesystem chrono iostreams log program_options test)
# install protobuf dependency
hunter_add_package(Protobuf)

include_directories(${CMAKE_INSTALL_INCLUDEDIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)
set(INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)

# set library target name
set(UTILITIES_TARGET "utilities")
set(CODEC_TARGET "codec")
set(PROTOCOL_TARGET "protocol")
# define the generated proto file path
set(PROTO_GENERATE_DIR ${INCLUDE_DIR}/bcos-framework/libprotocol/bcos-proto)

# add subdirectories to build the libraries
add_subdirectory(libutilities)
add_subdirectory(interfaces)
add_subdirectory(libcodec)
add_subdirectory(libprotocol)

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

include(InstallInclude)
# for doxygen
include(BuildDocs)

# for code coverage
if (COVERAGE)
    include(Coverage)
    config_coverage("'/usr*' '${CMAKE_SOURCE_DIR}/bcos-cmake-scripts*' '${CMAKE_SOURCE_DIR}/test/bcos-test*'")
endif ()
