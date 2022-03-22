SET (CMAKE_C_COMPILER             "clang")
SET (CMAKE_CXX_COMPILER            "clang++")


cmake_minimum_required(VERSION 3.0)

project(canhacker)

set(SOURCES
	src/main.cpp
	src/CANFrame.cpp
	src/CANbus.cpp
	src/CmdLineBuffer.cpp
	src/CmdLineHelp.cpp
	src/CmdLineMgr.cpp
	src/CmdLineRegistry.cpp
	src/GMLAN.cpp
	src/OBDII.cpp
	src/ODBIIsocket.cpp
	src/ScreenMgr.cpp

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
