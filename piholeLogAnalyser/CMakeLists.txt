# CMakeList.txt : CMake project for piholeLogAnalyser, include source and define
# project specific logic here.
#

cmake_minimum_required(VERSION 3.25)
set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

# Add source to this project's executable.
add_executable (${PROJECT_NAME} "piholeLogAnalyser.cpp" "piholeLogAnalyserDefs.h" "dbInterface.h" "DNSQuery.h" )

if (CMAKE_BUILD_TYPE STREQUAL "Debug")	# CMAKE_BUILD_TYPE "Debug"" is set in CMakePresets.json
	target_compile_definitions(${PROJECT_NAME} PRIVATE DEBUG=1)
	message(STATUS "Building ${PROJECT_NAME} with build type: ${CMAKE_BUILD_TYPE}.")
else()
	message(STATUS "Building ${PROJECT_NAME} with build type: ${CMAKE_BUILD_TYPE}.")
endif()


message(STATUS "CXX_STANDARD: ${CMAKE_CXX_STANDARD}")

include_directories(${CMAKE_SOURCE_DIR}/Grok++)
include_directories("/usr/include")			# includes <mariadb/conncpp.hpp> 


find_library(LIBMARIADB_LIBRARY mariadbcpp REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE grok++ ${LIBMARIADB_LIBRARY})
message(STATUS "Mariadb Library Found: ${LIBMARIADB_LIBRARY}")

#target_compile_options(${PROJECT_NAME} PRIVATE "-fexperimental-library")


#target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBMARIADB_LIBRARY})

# TODO: Add tests and install targets if needed.

# Copy the executable to the project source directory after building
#set(EXECUTABLE_NAME "${EXECUTABLE_OUTPUT_PATH}${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
#message(STATUS "Executable Name: ${EXECUTABLE_NAME}")
#add_custom_command(TARGET ${PROJECT_NAME}
#                   POST_BUILD
#                   COMMAND ${CMAKE_COMMAND} -E copy ${EXECUTABLE_NAME} ${PROJECT_SOURCE_DIR})

message(STATUS "Copy Patterns to Executable Output Path: ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "Read Patterns from Project Source Directory: ${PROJECT_SOURCE_DIR}/Grok++")

add_custom_command(TARGET ${PROJECT_NAME}
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/Grok++/grok-patterns ${CMAKE_CURRENT_BINARY_DIR} )
