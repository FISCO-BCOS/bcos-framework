file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

add_library(${CODEC_TARGET} ${SRC_LIST} ${HEADERS})
target_compile_options(${CODEC_TARGET} PRIVATE -Wno-error -Wno-unused-variable)

target_link_libraries(${CODEC_TARGET} PUBLIC ${UTILITIES_TARGET})