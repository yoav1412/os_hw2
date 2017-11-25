cmake_minimum_required(VERSION 3.9)
project(OS_hw2)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES myshell_main.c myshell.c myshell.h)
add_executable(OS_hw2 ${SOURCE_FILES})