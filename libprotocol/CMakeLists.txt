set(MESSAGES_PROTOS bcos-proto/Transaction.proto bcos-proto/TransactionReceipt.proto bcos-proto/BlockHeader.proto bcos-proto/Block.proto)
set(MESSAGES_SRCS)
set(MESSAGES_HDRS)
file(GLOB SRC_LIST "*.cpp")
file(GLOB HEADERS "*.h")

file(MAKE_DIRECTORY ${PROTO_GENERATE_DIR})

foreach(proto_file ${MESSAGES_PROTOS})
  get_filename_component(proto_file_abs ${proto_file} ABSOLUTE)
  get_filename_component(bcos_proto_abs "bcos-proto" ABSOLUTE)
  get_filename_component(basename ${proto_file} NAME_WE)
  set(generated_files ${PROTO_GENERATE_DIR}/${basename}.pb.cc ${PROTO_GENERATE_DIR}/${basename}.pb.h)

  list(APPEND MESSAGES_SRCS ${PROTO_GENERATE_DIR}/${basename}.pb.cc)
  list(APPEND MESSAGES_HDRS ${PROTO_GENERATE_DIR}/${basename}.pb.h)

  add_custom_command(
      OUTPUT ${generated_files}
      COMMAND protobuf::protoc --cpp_out ${PROTO_GENERATE_DIR} -I ${bcos_proto_abs} ${proto_file_abs}
      COMMENT "Generating ${generated_files} from ${proto_file_abs}"
      VERBATIM
  )
endforeach()

aux_source_directory(./protobuf SRC_LIST)
include_directories(./protobuf)

add_library(${PROTOCOL_TARGET} ${SRC_LIST} ${MESSAGES_SRCS} ${HEADERS})
target_compile_options(${PROTOCOL_TARGET} PRIVATE -Wno-error -Wno-unused-variable)

find_package(Protobuf CONFIG REQUIRED)
target_link_libraries(${PROTOCOL_TARGET} PUBLIC ${UTILITIES_TARGET} protobuf::libprotobuf ${UTILITIES_TARGET})
