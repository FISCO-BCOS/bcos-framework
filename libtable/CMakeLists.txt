file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

add_library(${TABLE_TARGET} ${SRC_LIST} ${HEADERS})
# target_compile_options(${TABLE_TARGET} PRIVATE -Wno-error -Wno-unused-variable)
find_package(tbb CONFIG QUIET REQUIRED)
find_package(Boost CONFIG QUIET REQUIRED log chrono system filesystem iostreams thread)
target_link_libraries(${TABLE_TARGET} PUBLIC Boost::log Boost::chrono Boost::iostreams Boost::filesystem Boost::thread tbb::tbb)
