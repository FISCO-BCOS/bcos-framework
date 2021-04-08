file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

add_library(${PROTOCOL_TARGET} ${SRC_LIST} ${HEADERS})
target_compile_options(${PROTOCOL_TARGET} PRIVATE -Wno-error -Wno-unused-variable)

find_package(tbb)
target_link_libraries(${PROTOCOL_TARGET} PUBLIC ${UTILITIES_TARGET} tbb::tbb)