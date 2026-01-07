cmake_minimum_required(VERSION 3.10)
project(minirpc)
message(STATUS "CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
message(STATUS "ThirdParty path = ${CMAKE_SOURCE_DIR}/ThirdParty/newmuduo/lib")

aux_source_directory(. SRC_LIST)
aux_source_directory(./zk SRC_LIST_ZK)

add_library(${PROJECT_NAME} SHARED ${SRC_LIST} ${SRC_LIST_ZK})


target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/zk/
    ${PROJECT_SOURCE_DIR}/
)
target_link_libraries(${PROJECT_NAME} zookeeper_mt newmuduo protobuf)