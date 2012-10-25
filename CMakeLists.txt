cmake_minimum_required(VERSION 2.8)

# project name
project(irrlamb)

# set default build type
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

# external libraries
find_package(Lua51 REQUIRED)
include_directories(${LUA_INCLUDE_DIR})
include_directories("/usr/local/include/bullet")

add_definitions("-DTIXML_USE_STL")

# add source files
FILE(GLOB SRC_CPP "source/*.cpp" "source/engine/*.cpp" "source/objects/*.cpp" "source/irrb/*.cpp" "source/stb_vorbis/*.c" "source/tinyxml/*.cpp")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})

add_executable(${CMAKE_PROJECT_NAME} ${SRC_CPP})
target_link_libraries(${CMAKE_PROJECT_NAME} ${LUA_LIBRARIES} BulletDynamics BulletCollision LinearMath Irrlicht sqlite3 tinyxml openal)
