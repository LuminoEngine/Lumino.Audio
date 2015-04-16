# e.g.) $ cmake -DLN_EXCEPTION_BACKTRACE=OFF ..


cmake_minimum_required (VERSION 2.8.7)
project (LuminoAudio)

set(LN_STATIC_LIB_OUTPUT_NAME "LuminoAudio")
set(LN_LINK_LIBS "")
set(LN_UNIT_TEST_LINK_LIBS "")

#------------------------------------------------------------------------------
# Options
#------------------------------------------------------------------------------

# Unicode char set.
option(LN_USE_UNICODE_CHAR_SET "Use unicode char set." ON)

# MSVC Runtime library.
option(MSVC_LINK_MULTI_THREAD_STATIC_RUNTIME "static runtime (/MT, /MTd) or DLL runtime (/MD, /MDd) or to link either." ON)

# Exceprion Backtrace.
option(LN_EXCEPTION_BACKTRACE "In some unix environment there is a possibility that can not be compiled." ON)

# Architecture.
#option(LN_BUILD_X64 "Architecture." OFF)

#------------------------------------------------------------------------------
# Apply options
#------------------------------------------------------------------------------
# Unicode char set.
if (LN_USE_UNICODE_CHAR_SET)
	add_definitions(-DUNICODE)
	add_definitions(-D_UNICODE)
endif()

find_package(OpenGL REQUIRED)

if (WIN32)

	#------------------------------------------------------
	# Make static lib name.
	#	{ProjectName}_{msvcVer}{Arch}_static_{Runtime}
	#	e.g)	LuminoMath_msvc120x86_static_MTd.lib
	if (MSVC_VERSION EQUAL 1400)
		set(LN_STATIC_LIB_OUTPUT_NAME "${LN_STATIC_LIB_OUTPUT_NAME}_msvc80")
	elseif (MSVC_VERSION EQUAL 1500)
		set(LN_STATIC_LIB_OUTPUT_NAME "${LN_STATIC_LIB_OUTPUT_NAME}_msvc90")
	elseif (MSVC_VERSION EQUAL 1600)
		set(LN_STATIC_LIB_OUTPUT_NAME "${LN_STATIC_LIB_OUTPUT_NAME}_msvc100")
	elseif (MSVC_VERSION EQUAL 1700)
		set(LN_STATIC_LIB_OUTPUT_NAME "${LN_STATIC_LIB_OUTPUT_NAME}_msvc110")
	elseif (MSVC_VERSION EQUAL 1800)
		set(LN_STATIC_LIB_OUTPUT_NAME "${LN_STATIC_LIB_OUTPUT_NAME}_msvc120")
	endif()
	
	# Architecture.
	# http://stackoverflow.com/questions/5334095/cmake-multiarchitecture-compilation
	if (${CMAKE_EXE_LINKER_FLAGS} MATCHES "/machine:x64")	# /machine:x64 
		set(LN_STATIC_LIB_OUTPUT_NAME "${LN_STATIC_LIB_OUTPUT_NAME}x64")
	else()
		set(LN_STATIC_LIB_OUTPUT_NAME "${LN_STATIC_LIB_OUTPUT_NAME}x86")
	endif()
	
	set(LN_STATIC_LIB_OUTPUT_NAME "${LN_STATIC_LIB_OUTPUT_NAME}_static")
	
	# MSVC Runtime library.
	if (MSVC_LINK_MULTI_THREAD_STATIC_RUNTIME)
		set(LN_STATIC_LIB_OUTPUT_NAME "${LN_STATIC_LIB_OUTPUT_NAME}_MT")
		# Overwrite options.
		foreach (flag
			CMAKE_C_FLAGS
			CMAKE_C_FLAGS_DEBUG
			CMAKE_C_FLAGS_RELEASE
			CMAKE_CXX_FLAGS
			CMAKE_CXX_FLAGS_DEBUG
			CMAKE_CXX_FLAGS_RELEASE)
			if (${flag} MATCHES "/MD")
				string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
			endif()
			if (${flag} MATCHES "/MDd")
				string(REGEX REPLACE "/MDd" "/MTd" ${flag} "${${flag}}")
			endif()
		endforeach()
	else()
		set(LN_STATIC_LIB_OUTPUT_NAME "${LN_STATIC_LIB_OUTPUT_NAME}_MD")
	endif()
	set(CMAKE_DEBUG_POSTFIX "d")
	# end   Make static lib name.
	#------------------------------------------------------
	
	# link librarys.
	#list(APPEND LN_LINK_LIBS "imagehlp.lib")
	
	# platform sources.
	#set(LN_PLATFORM_SOURCES
	#	src/IO/Win32/FileUtils_win32.cpp)

elseif (APPLE)
	#set(LN_UNIT_TEST_LINK_LIBS "pthread")
	
elseif (UNIX)
	#set(LN_UNIT_TEST_LINK_LIBS "pthread")
	
else()
	# Not support.
    message(FATAL_ERROR "No supported platform was detected.")
endif()

#------------------------------------------------------------------------------
# Files
#------------------------------------------------------------------------------
#--------------------------------------
# Source files.
set(LN_SOURCES
	external/libogg/src/bitwise.c
	external/libogg/src/framing.c
	external/libvorbis/lib/analysis.c
	#external/libvorbis/lib/barkmel.c
	external/libvorbis/lib/bitrate.c
	external/libvorbis/lib/block.c
	external/libvorbis/lib/codebook.c
	external/libvorbis/lib/envelope.c
	external/libvorbis/lib/floor0.c
	external/libvorbis/lib/floor1.c
	external/libvorbis/lib/info.c
	external/libvorbis/lib/lookup.c
	external/libvorbis/lib/lpc.c
	external/libvorbis/lib/lsp.c
	external/libvorbis/lib/mapping0.c
	external/libvorbis/lib/mdct.c
	external/libvorbis/lib/psy.c
	#external/libvorbis/lib/psytune.c
	external/libvorbis/lib/registry.c
	external/libvorbis/lib/res0.c
	external/libvorbis/lib/sharedbook.c
	external/libvorbis/lib/smallft.c
	external/libvorbis/lib/synthesis.c
	#external/libvorbis/lib/tone.c
	external/libvorbis/lib/vorbisenc.c
	external/libvorbis/lib/vorbisfile.c
	external/libvorbis/lib/window.c
	src/Audio/AudioDevice.cpp
	src/Audio/AudioStream.cpp
	src/Audio/WaveStream.cpp
	${LN_PLATFORM_SOURCES})
	
set(LN_INCLUDE_DIRECTORIES
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/external/libogg/include
	${PROJECT_SOURCE_DIR}/external/libvorbis/include
	${PROJECT_SOURCE_DIR}/external/libvorbis/lib
	${PROJECT_SOURCE_DIR}/external/Lumino.Core/include
	${PROJECT_SOURCE_DIR}/external/Lumino.Math/include)
include_directories(${LN_INCLUDE_DIRECTORIES})

#------------------------------------------------------------------------------
# Output
#------------------------------------------------------------------------------

#------------------------------
# external deps.
add_subdirectory(external/Lumino.Core)
add_subdirectory(external/Lumino.Math)
link_libraries(LuminoCore_Static)
link_libraries(LuminoMath_Static)

#--------------------------------------
# Static library.
add_library(${PROJECT_NAME}_Static STATIC ${LN_SOURCES})
set_target_properties(${PROJECT_NAME}_Static PROPERTIES OUTPUT_NAME ${LN_STATIC_LIB_OUTPUT_NAME})
target_link_libraries(${PROJECT_NAME}_Static ${LN_LINK_LIBS})
add_custom_command(TARGET ${PROJECT_NAME}_Static POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}_Static> "${PROJECT_SOURCE_DIR}/lib")

# Unit test.
#add_subdirectory(test)

# samples.
#add_subdirectory(sample)
