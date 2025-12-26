cmake_minimum_required(VERSION 3.10)
project(rpc_client)

aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})
target_include_directories(${PROJECT_NAME} PRIVATE
    ${PROJECT_SOURCE_DIR}/../../include
    ${CMAKE_SOURCE_DIR}/ThirdParty/newmuduo/include
)
target_link_directories(${PROJECT_NAME} PRIVATE 
    ${PROJECT_SOURCE_DIR}/../../lib
    ${CMAKE_SOURCE_DIR}/ThirdParty/newmuduo/lib
)
target_link_libraries(${PROJECT_NAME} minirpc newmuduo)