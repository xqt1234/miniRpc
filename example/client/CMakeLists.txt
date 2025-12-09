cmake_minimum_required(VERSION 3.10)
project(client)

aux_source_directory(. SRC_LIST)
add_executable(${PROJECT_NAME} ${SRC_LIST})