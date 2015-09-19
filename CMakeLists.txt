cmake_minimum_required(VERSION 3.0)

# define constants
add_definitions(-DGAME_VERSION="0.2.0")

# projects
project(irrlamb)
subdirs(tools)

# add extra find modules
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")

# set default build type
set(EXTRA_LIBS)

# windows only options
if(MSVC)
	set(FREETYPE_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/extlibs/include")
	set(LUA_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/extlibs/include/lua5.2")

	set(SQLITE_LIBRARIES debug ${PROJECT_SOURCE_DIR}/extlibs/sqlite3/sqlite3_d.lib optimized ${PROJECT_SOURCE_DIR}/extlibs/sqlite3/sqlite3.lib)
	set(SQLITE_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/extlibs/sqlite3/include")
	set(FREETYPE_LIBRARY debug ${PROJECT_SOURCE_DIR}/extlibs/freetype/freetype_d.lib optimized ${PROJECT_SOURCE_DIR}/extlibs/freetype/freetype.lib)
	set(LUA_LIBRARIES debug ${PROJECT_SOURCE_DIR}/extlibs/debug/lua52_d.lib optimized ${PROJECT_SOURCE_DIR}/extlibs/release/lua52.lib)

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	add_definitions("-D_CRT_SECURE_NO_WARNINGS")
	link_directories("${PROJECT_SOURCE_DIR}/extlibs/")

	# set output path
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/)
else()
	if(NOT CMAKE_BUILD_TYPE)
		set(CMAKE_BUILD_TYPE Release)
	endif()

	# x11
	find_package(X11 REQUIRED)
	set(EXTRA_LIBS ${EXTRA_LIBS} ${X11_X11_LIB} ${X11_Xrandr_LIB} ${X11_Xext_LIB} ${X11_Xxf86vm_LIB})
	include_directories(${X11_Xrandr_INCLUDE_PATH})
	include_directories(${X11_X11_INCLUDE_PATH})
	include_directories(${X11_xf86vmode_INCLUDE_PATH})

	# lua
	set(Lua_FIND_VERSION_EXACT TRUE)
	find_package(Lua 5.3 REQUIRED)
	if(NOT LUA_FOUND)
		message(SEND_ERROR "Could not find Lua 5.3!")
	endif()

	if(PANDORA)
		add_definitions("-DPANDORA")
		add_definitions("-DLUA_USE_LINUX")
	endif()

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

find_package(Freetype REQUIRED)
if(NOT FREETYPE_FOUND)
	message(SEND_ERROR "Could not find Freetype 2!")
endif()

find_package(OpenAL REQUIRED)
if(NOT OPENAL_FOUND)
	message(SEND_ERROR "Could not find OpenAL!")
endif()

find_package(Vorbis REQUIRED)
if(NOT VORBIS_FOUND)
	message(SEND_ERROR "Could not find libvorbis or libvorbisfile!")
endif()

find_package(Ogg REQUIRED)
if(NOT OGG_FOUND)
	message(SEND_ERROR "Could not find libogg!")
endif()

find_package(SQLite REQUIRED)
if(NOT SQLITE_FOUND)
	message(SEND_ERROR "Could not find SQLite3!")
endif()

# include paths
include_directories(${BULLET_INCLUDE_DIRS})
include_directories(${IRRLICHT_INCLUDE_DIRS})
include_directories(${OPENAL_INCLUDE_DIR})
include_directories(${VORBIS_INCLUDE_DIR})
include_directories(${OGG_INCLUDE_DIR})
include_directories(${LUA_INCLUDE_DIR})
include_directories(${SQLITE_INCLUDE_DIR})
include_directories(${FREETYPE_INCLUDE_DIRS})
include_directories("src")

# add source code
file(GLOB SRC_MAIN src/*.cpp src/*.h)
file(GLOB SRC_ENGINE src/engine/*.cpp src/engine/*.h)
file(GLOB SRC_OBJECTS src/objects/*.cpp src/objects/*.h)
file(GLOB SRC_IRRB src/irrb/*.cpp src/irrb/*.h)
file(GLOB SRC_FONT src/font/*.cpp src/font/*.h)
file(GLOB SRC_TINYXML src/tinyxml/*.cpp src/tinyxml/*.h)

file(GLOB SRC_ALL ${SRC_MAIN} ${SRC_ENGINE} ${SRC_OBJECTS} ${SRC_IRRB} ${SRC_TINYXML} ${SRC_FONT})

# add virtual folders
source_group(main FILES ${SRC_MAIN})
source_group(engine FILES ${SRC_ENGINE})
source_group(objects FILES ${SRC_OBJECTS})
source_group(irrb FILES ${SRC_IRRB})
source_group(font FILES ${SRC_FONT})
source_group(tinyxml FILES ${SRC_TINYXML})

# create exe
add_executable(${CMAKE_PROJECT_NAME} ${SRC_ALL} src/resource.rc)

# link libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
	${IRRLICHT_LIBRARIES}
	${BULLET_LIBRARIES}
	${OPENAL_LIBRARY}
	${LUA_LIBRARIES}
	${SQLITE_LIBRARIES}
	${FREETYPE_LIBRARIES}
	${VORBIS_LIBRARIES}
	${OGG_LIBRARIES}
	${EXTRA_LIBS}
	)

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
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/scripts DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/shaders DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/sounds DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(DIRECTORY ${PROJECT_SOURCE_DIR}/working/textures DESTINATION share/games/${CMAKE_PROJECT_NAME})
	install(FILES ${PROJECT_SOURCE_DIR}/deployment/irrlamb.png DESTINATION share/pixmaps)
	install(FILES ${PROJECT_SOURCE_DIR}/deployment/irrlamb.desktop DESTINATION share/applications)
	install(FILES ${PROJECT_SOURCE_DIR}/CHANGELOG DESTINATION share/doc/${CMAKE_PROJECT_NAME})
	install(FILES ${PROJECT_SOURCE_DIR}/LICENSE DESTINATION share/doc/${CMAKE_PROJECT_NAME})
	install(FILES ${PROJECT_SOURCE_DIR}/README DESTINATION share/doc/${CMAKE_PROJECT_NAME})

	# generate the script to launch the program
	configure_file(${PROJECT_SOURCE_DIR}/deployment/${CMAKE_PROJECT_NAME} ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles)
	install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${CMAKE_PROJECT_NAME} DESTINATION bin)
endif()
