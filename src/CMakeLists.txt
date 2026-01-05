cmake_minimum_required(VERSION 3.10)
project(minirpc)
message(STATUS "CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")
message(STATUS "ThirdParty path = ${CMAKE_SOURCE_DIR}/ThirdParty/newmuduo/lib")

aux_source_directory(. SRC_LIST)
aux_source_directory(./zk SRC_LIST_ZK)

add_library(${PROJECT_NAME} SHARED ${SRC_LIST} ${SRC_LIST_ZK})

# if(TARGET newmuduo)
#     message(STATUS "✅ Using external newmuduo target")
# else()
#     message(STATUS "⚠️  newmuduo target not found, using local library")
#     target_include_directories(${PROJECT_NAME} PUBLIC
#     ${CMAKE_SOURCE_DIR}/ThirdParty/newmuduo/include
#     )
#     target_link_directories(${PROJECT_NAME} PRIVATE 
#     ${CMAKE_SOURCE_DIR}/ThirdParty/newmuduo/lib
# )
# endif()
target_include_directories(${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/zk/
    ${PROJECT_SOURCE_DIR}/
)

#add_executable(${PROJECT_NAME} ${SRC_LIST} ${SRC_LIST_ZK})
#link_directories(${PROJECT_SOURCE_DIR}/../lib)
target_link_libraries(${PROJECT_NAME} zookeeper_mt newmuduo)