cmake_minimum_required(VERSION 2.8)

# project name
project(irrlamb)
subdirs(tools)

# add extra find modules
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/;${CMAKE_MODULE_PATH}")
include(AddPCH.cmake)

# set default build type
set(EXTRA_LIBS)

# windows only options
if(MSVC)
	SET(ENV{LUA_DIR} ${PROJECT_SOURCE_DIR}/extlibs/)
	set(LUA_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/extlibs/include/lua51)
	set(LUA_LIBRARIES debug ${PROJECT_SOURCE_DIR}/extlibs/lua51_d.lib optimized ${PROJECT_SOURCE_DIR}/extlibs/lua51.lib)
	set(SQLITE3_LIBRARIES debug ${PROJECT_SOURCE_DIR}/extlibs/sqlite3_d.lib optimized ${PROJECT_SOURCE_DIR}/extlibs/sqlite3.lib)
	set(SQLITE3_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/extlibs/include/sqlite")
	
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	add_definitions("-D_CRT_SECURE_NO_WARNINGS")
	link_directories("${PROJECT_SOURCE_DIR}/extlibs/")
	
	# set output path
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/)
else()
	if(NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE Release)
	endif()
	
	#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -H")
	set(SQLITE3_LIBRARIES sqlite3)
	set(SQLITE3_INCLUDE_DIR "/usr/include")
	
	# x11
	find_package(X11 REQUIRED)
	set(EXTRA_LIBS ${EXTRA_LIBS} ${X11_X11_LIB} ${X11_Xrandr_LIB} ${X11_Xext_LIB} ${X11_Xxf86vm_LIB})
	include_directories(${X11_Xrandr_INCLUDE_PATH})
	include_directories(${X11_X11_INCLUDE_PATH})
	include_directories(${X11_xf86vmode_INCLUDE_PATH})

	# set output path
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})
endif()

# find libraries
set(BULLET_ROOT "$ENV{BULLET_ROOT}")
find_package(Irrlicht 1.8 REQUIRED)
if(NOT IRRLICHT_FOUND)
	message(SEND_ERROR "Could not find Irrlicht!")
endif()

find_package(Bullet REQUIRED)
if(NOT BULLET_FOUND)
	message(SEND_ERROR "Could not find Bullet!")
endif()

find_package(OpenAL REQUIRED)
if(NOT OPENAL_FOUND)
	message(SEND_ERROR "Could not find OpenAL!")
endif()

find_package(Lua51 REQUIRED)
if(NOT LUA51_FOUND)
	message(SEND_ERROR "Could not find Lua 5.1!")
endif()

# include paths
include_directories(${BULLET_INCLUDE_DIRS})
include_directories(${IRRLICHT_INCLUDE_DIRS})
include_directories(${OPENAL_INCLUDE_DIR})
include_directories(${LUA_INCLUDE_DIR})
include_directories(${SQLITE3_INCLUDE_DIR})
include_directories("src")

# preprocessor definitions
add_definitions("-DTIXML_USE_STL")

# add source code
file(GLOB SRC_MAIN src/*.cpp src/*.h)
file(GLOB SRC_ENGINE src/engine/*.cpp src/engine/*.h)
file(GLOB SRC_OBJECTS src/objects/*.cpp src/objects/*.h)
file(GLOB SRC_IRRB src/irrb/*.cpp src/irrb/*.h)
file(GLOB SRC_STBVORBIS src/stb_vorbis/*.cpp src/stb_vorbis/*.h)
file(GLOB SRC_TINYXML src/tinyxml/*.cpp src/tinyxml/*.h)

file(GLOB SRC_ALL ${SRC_MAIN} ${SRC_ENGINE} ${SRC_OBJECTS} ${SRC_IRRB} ${SRC_STBVORBIS} ${SRC_TINYXML})

# add virtual folders
source_group(main FILES ${SRC_MAIN})
source_group(engine FILES ${SRC_ENGINE})
source_group(objects FILES ${SRC_OBJECTS})
source_group(irrb FILES ${SRC_IRRB})
source_group(stbvorbis FILES ${SRC_STBVORBIS})
source_group(tinyxml FILES ${SRC_TINYXML})

# create exe
add_executable(${CMAKE_PROJECT_NAME} ${SRC_ALL} src/resource.rc)
add_pch(${CMAKE_PROJECT_NAME} src/stdafx.cpp ${PROJECT_SOURCE_DIR}/src/stdafx.h)

# link libraries
target_link_libraries(${CMAKE_PROJECT_NAME} ${IRRLICHT_LIBRARIES} ${BULLET_LIBRARIES} ${OPENAL_LIBRARY} ${LUA_LIBRARIES} ${SQLITE3_LIBRARIES})

# copy irrlicht DLLs to working directory
if(WIN32)
	add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "$ENV{IRRLICHT_ROOT}/bin/Win32-VisualStudio/Irrlicht.dll" ${PROJECT_SOURCE_DIR}/working)
	add_custom_command(TARGET ${CMAKE_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "$ENV{IRRLICHT_ROOT}/bin/Win32-VisualStudio/Irrlicht_d.dll" ${PROJECT_SOURCE_DIR}/working)
else()
	
	# linux installation
	install(TARGETS ${CMAKE_PROJECT_NAME} RUNTIME DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/art DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/fonts DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/levels DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/meshes DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/scenes DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/scripts DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/shaders DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/sounds DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/textures DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(FILES ${PROJECT_SOURCE_DIR}/deployment/irrlamb.xpm DESTINATION share/pixmaps)
	install(FILES ${PROJECT_SOURCE_DIR}/deployment/irrlamb.desktop DESTINATION share/applications)
	install(FILES ${PROJECT_SOURCE_DIR}/deployment/license.txt DESTINATION share/doc/${CMAKE_PROJECT_NAME})
	install(FILES ${PROJECT_SOURCE_DIR}/deployment/changelog.txt DESTINATION share/doc/${CMAKE_PROJECT_NAME})
	install(FILES ${PROJECT_SOURCE_DIR}/README DESTINATION share/doc/${CMAKE_PROJECT_NAME})
	install(PROGRAMS ${PROJECT_SOURCE_DIR}/deployment/irrlamb DESTINATION games)
endif()

#get_cmake_property(vars VARIABLES)
#foreach(vars ${vars})
#    message(STATUS "${vars}=${${vars}}")
#endforeach()