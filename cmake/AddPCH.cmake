# Alan Witkowski - https://github.com/jazztickets
# This macro adds a precompiled header to the project.
# It currently doesn't support mixing C/C++ files together
#
# target    - name of target to apply pch to (e.g. ${CMAKE_PROJECT_NAME} )
# sourcecpp - path to pch source file (e.g. src/all.cpp) 
# header    - path to pch header file (e.g. src/all.h) 
macro(add_pch target sourcecpp header)

	# Extract header filename from path
	get_filename_component(headerfile ${header} NAME)

	# Handle visual studio.
	if(MSVC)
		set_target_properties(${target} PROPERTIES COMPILE_FLAGS "/Yu${headerfile}")
		set_source_files_properties(${sourcecpp} PROPERTIES COMPILE_FLAGS "/Yc${headerfile}")
	
	# Handle GCC
	elseif(UNIX)
		
		# Get compiler flags from build type
		string(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" compiler_flags)
		set(compiler_flags ${${compiler_flags}})
		separate_arguments(compiler_flags)

		# Build list of include dirs separated by -I
		get_directory_property(include_dirs DIRECTORY ${CMAKE_SOURCE_DIR} INCLUDE_DIRECTORIES)
		foreach(path ${include_dirs})
			set(include_flag "${include_flag}-I${path} ")
		endforeach()
		separate_arguments(include_flag)

		# Get definitions
		get_directory_property(compile_defs DIRECTORY ${CMAKE_SOURCE_DIR} COMPILE_DEFINITIONS)
		foreach(flag ${compile_defs})
			set(compiledef_flag "${compiledef_flag}-D${flag} ")
		endforeach()
		separate_arguments(compiledef_flag)

		# Add a dependency to main target
		add_dependencies(${target} ${target}_pre)
		
		# Create a target that depends on the pch
		add_custom_target(${target}_pre DEPENDS ${EXECUTABLE_OUTPUT_PATH}/${headerfile}.gch)
		
		# Workaround for cmake not handling spaces in CMAKE_CXX_COMPILER correctly
		set(_compiler "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1}")
		separate_arguments(_compiler)
		
		# Create command to generate pch
		add_custom_command(
			OUTPUT ${EXECUTABLE_OUTPUT_PATH}/${headerfile}.gch
			COMMAND ${_compiler} ${compiledef_flag} ${include_flag} -xc++-header ${header} -o ${EXECUTABLE_OUTPUT_PATH}/${headerfile}.gch ${compiler_flags}
			DEPENDS ${header}
		)
		
		# Create symlink for headerfile
		add_custom_command(
			TARGET ${target}_pre
			COMMAND ${CMAKE_COMMAND} -E create_symlink ${header} ${EXECUTABLE_OUTPUT_PATH}/${headerfile}
			DEPENDS ${header}
		)
		
		# Add pch directory to include path
		include_directories(BEFORE ${EXECUTABLE_OUTPUT_PATH})
	endif()
	
endmacro()
