aux_source_directory(. DIR_SRCS)

add_library(base ${DIR_SRCS})
target_link_libraries(base glog pthread)

