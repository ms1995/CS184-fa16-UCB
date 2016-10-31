cmake_minimum_required(VERSION 2.8)
project(assignment2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SOURCE_FILES main.cpp Vec3.h Vec3.cpp Sphere.h Sphere.cpp Light.h Mat4.cpp Mat4.h Triangle.cpp Triangle.h Utils.h Material.h Mat3.cpp Mat3.h)

add_executable(assignment2 ${SOURCE_FILES})

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	if (${CMAKE_SIZEOF_VOID_P} MATCHES "8")
		file(GLOB LIBRARIES libjpeg/lib/linux_x64.a)
	endif()
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	file(GLOB LIBRARIES libjpeg/lib/mac_x64.a)
endif()

target_link_libraries(assignment2 ${LIBRARIES} pthread)
