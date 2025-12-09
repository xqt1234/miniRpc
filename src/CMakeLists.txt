cmake_minimum_required(VERSION 3.10)
project(minirpc)

aux_source_directory(. SRC_LIST)
add_library(${PROJECT_NAME} SHARED ${SRC_LIST})