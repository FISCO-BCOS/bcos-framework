file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

add_library(${UTILITIES_TARGET} ${SRC_LIST} ${HEADERS})
target_compile_options(${UTILITIES_TARGET} PRIVATE -Wno-error -Wno-unused-variable)

find_package(Boost CONFIG REQUIRED log system filesystem iostreams thread)
target_link_libraries(${UTILITIES_TARGET} PUBLIC Boost::log Boost::iostreams Boost::filesystem Boost::thread)