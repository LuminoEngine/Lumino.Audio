set(LN_TEST_SOURCES
	UnitTest/Main.cpp)

include_directories(
	"${PROJECT_SOURCE_DIR}/include"
	"${PROJECT_SOURCE_DIR}/external/LuminoCore/include"
	"${PROJECT_SOURCE_DIR}/external/LuminoMath/include"
	"${PROJECT_SOURCE_DIR}/test/gtest/include"
	"${PROJECT_SOURCE_DIR}/test/gtest"
	"${PROJECT_SOURCE_DIR}/test/gtest/src"
	"${PROJECT_SOURCE_DIR}/test/UnitTest")

set(LN_TEST_LIBS LuminoAudio_Static)
if (WIN32)
elseif (APPLE)
	list(APPEND LN_TEST_LIBS "pthread")	# for gtest.
elseif (UNIX)
	list(APPEND LN_TEST_LIBS "pthread")	# for gtest.
else()
endif()
link_libraries(${LN_TEST_LIBS})

add_definitions(-DGTEST_HAS_TR1_TUPLE=0)	# vs2012

add_executable(LuminoAudio_UnitTest ${LN_TEST_SOURCES})
add_dependencies(LuminoAudio_UnitTest LuminoAudio_Static)
