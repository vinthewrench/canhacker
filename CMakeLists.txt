SET (CMAKE_C_COMPILER             "clang")
SET (CMAKE_CXX_COMPILER            "clang++")


cmake_minimum_required(VERSION 3.0)

project(obdtool)

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

add_executable(obdtool ${SOURCES})


set_target_properties(obdtool PROPERTIES
				CXX_STANDARD 17
				CXX_EXTENSIONS OFF
				)
 
set_target_properties(obdtool PROPERTIES
			OUTPUT_NAME "obdtool"
			)

			  
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(/usr/include)
link_directories(/usr/lib)
target_link_libraries(obdtool  PRIVATE Threads::Threads)

target_include_directories(obdtool
	PRIVATE
	src
)

set(CMAKE_BINARY_DIR "bin")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
