cmake_minimum_required(VERSION 2.8.11)

file(GLOB_RECURSE RAZTER_HEADERS "${CMAKE_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE RAZTER_C *.c)

if(APPLE)
	file(GLOB_RECURSE RAZTER_M *.m)
	add_library(razter ${RAZTER_HEADERS} ${RAZTER_M} ${RAZTER_C})
else()
	add_library(razter ${RAZTER_HEADERS} ${RAZTER_C})
endif()


target_include_directories (razter PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

target_link_libraries (razter glfw ${GLFW_LIBRARIES})