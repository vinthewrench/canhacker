SET (CMAKE_C_COMPILER             "clang")
SET (CMAKE_CXX_COMPILER            "clang++")


cmake_minimum_required(VERSION 3.0)

project(canhacker)

set(SOURCES
	src/main.cpp
	src/CanBusMgr.cpp
	src/CmdLineBuffer.cpp
	src/CmdLineMgr.cpp
	src/CmdLineRegistry.cpp
	src/FrameDB.cpp
	src/FrameDumper.cpp
	src/GMLAN.cpp
	src/OBD2.cpp
	src/Wranger2010.cpp
	)

add_executable(canhacker ${SOURCES})


set_target_properties(canhacker PROPERTIES
				CXX_STANDARD 17
				CXX_EXTENSIONS OFF
				)
 
set_target_properties(canhacker PROPERTIES
			OUTPUT_NAME "canhacker"
			)

			  
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(/usr/include)
link_directories(/usr/lib)
target_link_libraries(canhacker  PRIVATE Threads::Threads)

target_include_directories(canhacker
	PRIVATE
	src
)

set(CMAKE_BINARY_DIR "bin")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
