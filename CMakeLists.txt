cmake_minimum_required(VERSION 2.8)

# project name
project(irrlamb)

# add extra find modules
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/;${CMAKE_MODULE_PATH}")

# set default build type
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

# windows only options
if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	
	# set output path
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/)
else()

	# set output path
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
endif()

# find libraries
set(BULLET_ROOT "$ENV{BULLET_ROOT}")
find_package(Irrlicht REQUIRED)
find_package(OpenAL REQUIRED)
find_package(Lua51 REQUIRED)
find_package(Bullet REQUIRED)

# include paths
include_directories(${BULLET_INCLUDE_DIRS})
include_directories(${IRRLICHT_INCLUDE_DIRS})
include_directories(${LUA_INCLUDE_DIR})
include_directories(${OPENAL_INCLUDE_DIR})
include_directories("src/sqlite")

# preprocessor definitions
add_definitions("-DTIXML_USE_STL")

# add source code
file(GLOB SRC_MAIN src/*.cpp src/*.h)
file(GLOB SRC_ENGINE src/engine/*.cpp src/engine/*.h)
file(GLOB SRC_OBJECTS src/objects/*.cpp src/objects/*.h)
file(GLOB SRC_IRRB src/irrb/*.cpp src/irrb/*.h)
file(GLOB SRC_SQLITE src/sqlite/*.c src/sqlite/*.h)
file(GLOB SRC_STBVORBIS src/stb_vorbis/*.c src/stb_vorbis/*.h)
file(GLOB SRC_TINYXML src/tinyxml/*.cpp src/tinyxml/*.h)

file(GLOB SRC_ALL ${SRC_MAIN} ${SRC_ENGINE} ${SRC_OBJECTS} ${SRC_IRRB} ${SRC_SQLITE} ${SRC_STBVORBIS} ${SRC_TINYXML})

# add virtual folders
source_group(main FILES ${SRC_MAIN})
source_group(engine FILES ${SRC_ENGINE})
source_group(objects FILES ${SRC_OBJECTS})
source_group(irrb FILES ${SRC_IRRB})
source_group(sqlite FILES ${SRC_SQLITE})
source_group(stbvorbis FILES ${SRC_STBVORBIS})
source_group(tinyxml FILES ${SRC_TINYXML})

# create exe
add_executable(${CMAKE_PROJECT_NAME} ${SRC_ALL})

# link libraries
target_link_libraries(${CMAKE_PROJECT_NAME} ${LUA_LIBRARIES} ${BULLET_LIBRARIES} ${IRRLICHT_LIBRARIES} ${OPENAL_LIBRARY})

# copy ogre DLLs to working directory
if(WIN32)
	add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "$ENV{IRRLICHT_ROOT}/bin/Win32-VisualStudio/Irrlicht.dll" ${PROJECT_SOURCE_DIR}/working)
endif()