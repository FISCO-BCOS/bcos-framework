aux_source_directory(. SRC_LIST)
file(GLOB HEADERS "*.h")

aux_source_directory(./libcrypto SRC_LIST)
include_directories(./libcrypto)