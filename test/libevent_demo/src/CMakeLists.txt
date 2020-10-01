cmake_minimum_required(VERSION 3.5)

project(myLibevent)

# message("top: CMAKE_SOURCE_DIR = ${CMAKE_SOURCE_DIR}")

file(GLOB USER_LIBS_PATH ../lib/*.c)

add_executable(libeventTest ./../src/main.c)

add_library(event
    STATIC # 编译库的类型 SHARED STATIC
    ${USER_LIBS_PATH}
    )
 
target_link_libraries(libeventTest
    PUBLIC event)

target_include_directories(libeventTest PUBLIC ./../lib)


message(STATUS ${USER_LIBS_PATH})
