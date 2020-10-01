
# 针对cmake版本的要求
cmake_minimum_required(VERSION 3.5)

project(myLibevntTest)


set(FILE_DESC_TEST fileDescTest)
add_executable(${FILE_DESC_TEST} main.c)

file(GLOB USER_LIBS_FILE ./../lib/*.c)

add_library(event
    STATIC #将库编译成静态库
    ${USER_LIBS_FILE})       

message(STATUS ${USER_LIBS_FILE})

target_link_libraries(${FILE_DESC_TEST} 
    PUBLIC event)

target_include_directories(${FILE_DESC_TEST} PUBLIC ./../lib)

