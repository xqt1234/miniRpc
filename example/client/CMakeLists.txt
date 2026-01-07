cmake_minimum_required(VERSION 3.10)
project(rpc_client)

aux_source_directory(. SRC_LIST)
aux_source_directory(../common SRC_LIST2)
add_executable(${PROJECT_NAME} ${SRC_LIST} ${SRC_LIST2})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/../common/
)

target_link_libraries(${PROJECT_NAME} minirpc newmuduo)