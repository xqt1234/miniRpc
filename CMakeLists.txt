cmake_minimum_required(VERSION 3.10)
project(myrpc)
set(LIBRARY_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++20 -fPIC -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++20 -fPIC -fsanitize=thread")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++20")
# include_directories(${PROJECT_SOURCE_DIR}/src)


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#add_subdirectory(src/server)
file(GLOB HEADER_FILES 
     ${PROJECT_SOURCE_DIR}/src/*.h
     ${PROJECT_SOURCE_DIR}/src/zk/*.h)
if(HEADER_FILES)
    file(COPY ${HEADER_FILES} DESTINATION ${PROJECT_SOURCE_DIR}/include/)
endif()
if(NOT TARGET newmuduo)
    add_subdirectory(ThirdParty/newmuduo)
endif()
add_subdirectory(src)
add_subdirectory(example)