# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.25)
# requires mariadb-connector-cpp 3.4.0 or higher (which requires mariadb-connector-c 3.4.0 or higher)
# requires cmake on the remote system to be at least version 3.25 to support the FetchContent module for downloading and building the GoogleTest framework for unit testing in Debug builds.
# requires gcc version 13 or above for C++20 support to support the latest C++ standards and features used in the project.
# sudo apt install -y openssh-server build-essential gdb rsync make zip


# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")
project ("piholeLogAnalyser" CXX)

message(STATUS "C Compiler: ${CMAKE_C_COMPILER}")
message(STATUS "C++ Compiler: ${CMAKE_CXX_COMPILER}")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Include sub-projects.
add_subdirectory ("piholeLogAnalyser")
add_subdirectory ("Grok++")

if (CMAKE_BUILD_TYPE STREQUAL "Debug")	# CMAKE_BUILD_TYPE "Debug"" is set in CMakePresets.json
	add_subdirectory ("LogAnalyserUnitTests")
	add_subdirectory ("Grok++UnitTests")
endif()


