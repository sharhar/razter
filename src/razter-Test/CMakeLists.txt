cmake_minimum_required(VERSION 2.8.11)

file(GLOB_RECURSE RAZTER_TEST_SRC *.cpp)

if(APPLE)
	add_executable(
		razter-Test 
		MACOSX_BUNDLE 
		${RAZTER_TEST_SRC}
	)
else()
	add_executable(razter-Test ${RAZTER_TEST_SRC})
endif()

target_link_libraries (razter-Test LINK_PUBLIC ${OPENGL_LIBRARIES})
target_link_libraries (razter-Test LINK_PUBLIC VKL)
target_link_libraries (razter-Test glfw ${GLFW_LIBRARIES})
