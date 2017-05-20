cmake_minimum_required(VERSION 2.8.11)

file(GLOB_RECURSE RAZTER_HEADERS "${CMAKE_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE RAZTER_CPP *.cpp)
file(GLOB_RECURSE RAZTER_C *.c)

add_library(razter ${RAZTER_HEADERS} ${RAZTER_CPP} ${RAZTER_C})

target_include_directories (razter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})