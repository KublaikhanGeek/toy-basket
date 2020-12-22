include_directories(
  ${PROJECT_SOURCE_DIR}/src/base
  ${PROJECT_SOURCE_DIR}/src/communication/net
  ${PROJECT_SOURCE_DIR}/src/communication)

aux_source_directory(. DIR_SRCS)

file(GLOB_RECURSE SRC_FILES 
 ${PROJECT_SOURCE_DIR}/src/communication/net/*.cpp
 ${PROJECT_SOURCE_DIR}/src/communication/serial/*.cpp
 ${PROJECT_SOURCE_DIR}/src/communication/net/protorpc/*.cc)

link_directories(${PROJECT_SOURCE_DIR}/lib)

add_library(communication ${DIR_SRCS} ${SRC_FILES})
target_link_libraries(communication glog pthread)
