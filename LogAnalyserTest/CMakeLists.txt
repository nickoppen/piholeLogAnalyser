
cmake_minimum_required(VERSION 3.25)

project(LogAnalyserTest)
enable_testing()

include(FetchContent)
set(GOOGLETEST_VERSION "1.17.0")
FetchContent_Declare(googleTest
	URL	"https://github.com/google/googletest/archive/refs/tags/v${GOOGLETEST_VERSION}.tar.gz"
	FIND_PACKAGE_ARGS ${GOOGLETEST_VERSION})

	FetchContent_MakeAvailable(googleTest)

add_executable(${PROJECT_NAME} "test.cpp" )

if (CMAKE_BUILD_TYPE STREQUAL "Debug")	# CMAKE_BUILD_TYPE "Debug"" is set in CMakePresets.json
	target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG=1)
endif()

include_directories(${CMAKE_SOURCE_DIR}/Grok++)
include_directories(${CMAKE_SOURCE_DIR}/piholeLogAnalyser)

include_directories("/usr/include")	

find_library(LIBMARIADB_LIBRARY mariadbcpp REQUIRED)
#target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBMARIADB_LIBRARY})
target_link_libraries(${PROJECT_NAME} GTest::gtest_main ${LIBMARIADB_LIBRARY}) # Automatically handles include paths

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})