cmake_minimum_required(VERSION 2.8.3)
project(teleop_twist_keyboard_cpp)

set(CMAKE_THREAD_LIBS_INIT "-lpthread")
set(CMAKE_HAVE_THREADS_LIBRARY 1)
set(CMAKE_USE_WIN32_THREADS_INIT 0)
set(CMAKE_USE_PTHREADS_INIT 1)
set(THREADS_PREFER_PTHREAD_FLAG ON)

set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

find_package(catkin REQUIRED COMPONENTS
  roscpp
  geometry_msgs
  #Threads REQUIRED #look into this
)

catkin_package(
  INCLUDE_DIRS src
  CATKIN_DEPENDS roscpp geometry_msgs
)

include_directories(
	${catkin_INCLUDE_DIRS})

#add_library(pthread SHARED IMPORTED
#	     c       SHARED IMPORTED)
set_target_properties(pthread PROPERTIES IMPORTED_LOCATION /usr/lib/x86_64-linux-gnu/libpthread.a
#		       c       PROPERTIES IMPORTED_LOCATION /usr/lib/x86_64-linux-gnu/libc.a)

add_executable(       teleop_twist_keyboard src/teleop_twist_keyboard.cpp src/global_key_variables.cpp)
target_link_libraries(teleop_twist_keyboard ${catkin_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT}
	/usr/local/openvibe-2.2.0-src/dependencies/lib/libvrpn.a
	/usr/local/openvibe-2.2.0-src/dependencies/lib/libvrpnserver.a
	pthread
	c)
	# /usr/lib/x86_64-linux-gnu/libpthread.a
	# /usr/lib/x86_64-linux-gnu/libc.a)

