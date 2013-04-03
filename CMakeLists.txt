cmake_minimum_required(VERSION 2.8)

# project name
project(irrlamb)
subdirs(tools)

# add extra find modules
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/;${CMAKE_MODULE_PATH}")

# set default build type
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Release)
endif()

set(EXTRA_LIBS)

# windows only options
if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
	
	# set output path
	set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/)
else()
	
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
find_package(OpenAL REQUIRED)
find_package(Bullet REQUIRED)

# include paths
include_directories(${BULLET_INCLUDE_DIRS})
include_directories(${IRRLICHT_INCLUDE_DIRS})
include_directories(${OPENAL_INCLUDE_DIR})
include_directories("src/lua")
include_directories("src/sqlite")

# preprocessor definitions
add_definitions("-DTIXML_USE_STL")

# add source code
file(GLOB SRC_MAIN src/*.cpp src/*.h)
file(GLOB SRC_ENGINE src/engine/*.cpp src/engine/*.h)
file(GLOB SRC_OBJECTS src/objects/*.cpp src/objects/*.h)
file(GLOB SRC_IRRB src/irrb/*.cpp src/irrb/*.h)
file(GLOB SRC_SQLITE src/sqlite/*.c src/sqlite/*.h)
file(GLOB SRC_LUA src/lua/*.c src/lua/*.h)
file(GLOB SRC_STBVORBIS src/stb_vorbis/*.c src/stb_vorbis/*.h)
file(GLOB SRC_TINYXML src/tinyxml/*.cpp src/tinyxml/*.h)

file(GLOB SRC_ALL ${SRC_MAIN} ${SRC_ENGINE} ${SRC_OBJECTS} ${SRC_IRRB} ${SRC_SQLITE} ${SRC_LUA} ${SRC_STBVORBIS} ${SRC_TINYXML})

# add virtual folders
source_group(main FILES ${SRC_MAIN})
source_group(engine FILES ${SRC_ENGINE})
source_group(objects FILES ${SRC_OBJECTS})
source_group(irrb FILES ${SRC_IRRB})
source_group(sqlite FILES ${SRC_SQLITE})
source_group(lua FILES ${SRC_LUA})
source_group(stbvorbis FILES ${SRC_STBVORBIS})
source_group(tinyxml FILES ${SRC_TINYXML})

# create exe
add_executable(${CMAKE_PROJECT_NAME} ${SRC_ALL} src/resource.rc)

# link libraries
target_link_libraries(${CMAKE_PROJECT_NAME} ${IRRLICHT_LIBRARIES} ${BULLET_LIBRARIES} ${OPENAL_LIBRARY})

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