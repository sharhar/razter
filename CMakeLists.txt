CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)

PROJECT(razter)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_VERBOSE_MAKEFILE ON)

list (APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

if (WIN32)
    add_definitions (-D_WIN32_WINNT=0x0602 -DWINVER=0x0602)

    find_package (DirectX)
endif()


if(NOT MSVC)
	include(CheckCXXCompilerFlag)
	CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
	CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
	if(COMPILER_SUPPORTS_CXX11)
		message(STATUS "Found CXX11 support in ${CMAKE_CXX_COMPILER}")
	    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	elseif(COMPILER_SUPPORTS_CXX0X)
		message(STATUS "Found CXX0X support in ${CMAKE_CXX_COMPILER}")
	    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
	else()
		message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
	endif()

endif()

option(GLUI_BUILD_TEST OFF)

include_directories(
	"${CMAKE_SOURCE_DIR}/VKL/include"
)

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/VKL")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/razter")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/src/razter-Test")