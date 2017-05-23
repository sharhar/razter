cmake_minimum_required(VERSION 2.8.11)

file(GLOB_RECURSE RAZTER_TEST_H *.h)
file(GLOB_RECURSE RAZTER_TEST_C *.c)
file(GLOB_RECURSE RAZTER_TEST_CPP *.cpp)

if(APPLE)
	add_executable(
		razter-Test 
		MACOSX_BUNDLE 
		${RAZTER_TEST_SRC}
	)
else()
	add_executable(razter-Test ${RAZTER_TEST_H} ${RAZTER_TEST_C} ${RAZTER_TEST_CPP})
endif()

target_include_directories (razter-Test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../include)
target_include_directories (razter-Test PUBLIC ${DirectX_D3D11_INCLUDE_DIR})
target_include_directories (razter-Test PUBLIC ${DirectX_D3DX11_INCLUDE_DIR})

target_link_libraries (razter-Test LINK_PUBLIC VKL)
target_link_libraries (razter-Test LINK_PUBLIC razter)
target_link_libraries (razter-Test glfw ${GLFW_LIBRARIES})
target_link_libraries (razter-Test LINK_PUBLIC ${DirectX_D3DX11_LIBRARY})
target_link_libraries (razter-Test LINK_PUBLIC ${DirectX_D3D11_LIBRARY})
