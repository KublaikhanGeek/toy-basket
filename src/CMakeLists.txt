
add_subdirectory(base)
add_subdirectory(communication)

include_directories(
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}/src/base
  ${PROJECT_SOURCE_DIR}/src/communication)

aux_source_directory(. DIR_SRCS)

file(GLOB_RECURSE SRC_FILES 
 ${PROJECT_SOURCE_DIR}/src/task/*.cpp 
 ${PROJECT_SOURCE_DIR}/src/componet/*.cpp)


#link_directories(${PROJECT_SOURCE_DIR}/lib)
add_executable(main ${DIR_SRCS} ${SRC_FILES})
target_link_libraries(main base communication glog cjson pthread)
