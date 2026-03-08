cmake_minimum_required(VERSION 3.13)
project(json)


add_library(json INTERFACE)
target_include_directories(json INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/")
