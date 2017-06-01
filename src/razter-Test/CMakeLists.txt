cmake_minimum_required(VERSION 2.8.11)

file(GLOB_RECURSE RAZTER_TEST_H *.h)
file(GLOB_RECURSE RAZTER_TEST_C *.c)

if(APPLE)

	file(GLOB_RECURSE RAZTER_TEST_M *.m)
	#file(GLOB_RECURSE RAZTER_TEST_METAL *.metal)

	add_executable(
		razter-Test 
		MACOSX_BUNDLE 
		${RAZTER_TEST_H} 
		${RAZTER_TEST_C} 
		${RAZTER_TEST_M} 
		#${RAZTER_TEST_METAL}
		"${CMAKE_CURRENT_SOURCE_DIR}/res"
	)

	#set_source_files_properties(${RAZTER_TEST_METAL} PROPERTIES LANGUAGE METAL)	

	SET_SOURCE_FILES_PROPERTIES(
		"${CMAKE_CURRENT_SOURCE_DIR}/res"
		PROPERTIES
		MACOSX_PACKAGE_LOCATION Resources
		)

elseif(MSVC)
	add_executable(razter-Test ${RAZTER_TEST_H} ${RAZTER_TEST_C})
	file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/res" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Debug")
	file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/res" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Release")
	file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/res" DESTINATION "${CMAKE_BINARY_DIR}/src/razter-Test")
else()
	add_executable(razter-Test ${RAZTER_TEST_H} ${RAZTER_TEST_C})
	file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/res" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
endif()

target_include_directories (razter-Test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)

if(APPLE)
	find_path(QUARTZCORE_INCLUDE_DIR QuartzCore/QuartzCore.h)
	find_library(QUARTZCORE_LIBRARY QuartzCore)

	find_path(METAL_INCLUDE_DIR Metal/Metal.h)
	find_library(METAL_LIBRARY Metal)

	target_link_libraries (razter-Test ${QUARTZCORE_LIBRARY})
	target_link_libraries (razter-Test ${METAL_LIBRARY})
endif()

target_link_libraries (razter-Test LINK_PUBLIC VKL)
target_link_libraries (razter-Test LINK_PUBLIC razter)
target_link_libraries (razter-Test glfw ${GLFW_LIBRARIES})
