include(ExternalProject)
include(GNUInstallDirs)

if (CMAKE_SYSTEM_NAME MATCHES "Darwin")
    set(PROTOBUF_CONFIG_COMMAND sh ./configure --prefix=${CMAKE_SOURCE_DIR}/deps)
else()
    set(PROTOBUF_CONFIG_COMMAND bash configure --prefix=${CMAKE_SOURCE_DIR}/deps)
endif ()

set(PROTOBUF_BUILD_COMMAND make)
set(PROTOBUF_INSTALL_COMMAND make install)
ExternalProject_Add(protobuf
        PREFIX ${CMAKE_SOURCE_DIR}/deps
        DOWNLOAD_NAME protobuf-cpp.tar.gz
        DOWNLOAD_NO_PROGRESS 1
        URL https://github.com/protocolbuffers/protobuf/releases/download/v3.14.0/protobuf-cpp-3.14.0.tar.gz
        URL_HASH SHA256=50ec5a07c0c55d4ec536dd49021f2e194a26bfdbc531d03d1e9d4d3e27175659
        BUILD_IN_SOURCE 1
        CONFIGURE_COMMAND ${PROTOBUF_CONFIG_COMMAND}
        LOG_CONFIGURE 1
        LOG_BUILD 1
        LOG_INSTALL 1
        BUILD_COMMAND ${PROTOBUF_BUILD_COMMAND}
        INSTALL_COMMAND ${PROTOBUF_INSTALL_COMMAND}
)

ExternalProject_Get_Property(protobuf SOURCE_DIR)
add_library(PROTOBUF STATIC IMPORTED)
set(PROTOBUF_SUFFIX .a)
set(PROTOBUF_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/deps/include)
set(PROTOBUF_LIBRARY ${CMAKE_SOURCE_DIR}/deps/lib/libprotobuf.a)
set(PROTOBUF_LIBRARIES ${PROTOBUF_LIBRARY})

file(MAKE_DIRECTORY ${PROTOBUF_INCLUDE_DIRS})  # Must exist.
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/deps/lib/)  # Must exist.

set_property(TARGET PROTOBUF PROPERTY IMPORTED_LOCATION ${PROTOBUF_LIBRARIES})
set_property(TARGET PROTOBUF PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${PROTOBUF_INCLUDE_DIRS})

add_dependencies(PROTOBUF protobuf)

message(STATUS "protobuf include  : ${PROTOBUF_INCLUDE_DIRS}")
message(STATUS "protobuf libraries: ${PROTOBUF_LIBRARY}")
