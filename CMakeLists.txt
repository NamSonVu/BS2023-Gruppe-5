cmake_minimum_required(VERSION 3.7.2)
project(TestRM C)
include_directories( ./src )
set(CMAKE_C_STANDARD 99)


add_executable(TestRM main.c keyValStore.c sub.c )

target_link_libraries(TestRM pthread)


