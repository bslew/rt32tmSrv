cmake_minimum_required (VERSION 2.6)

################################################################
################################################################
################################################################
# Set project name
#
project (rt32tmSrv)
set(CMAKE_CXX_STANDARD 17)


################################################################
################################################################
################################################################
# Set version number
#

set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 2)
set(VERSION_STRING ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

set (RT32TMSRV_VERSION ${RT32TMSRV_VERSION_STRING})

# Set build type

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()



################################################################
################################################################
################################################################
# Find required packages
#

find_package(Boost REQUIRED)
#if (Boost_FOUND)
	MESSAGE(STATUS "Boost_INCLUDE_DIR: " ${Boost_INCLUDE_DIR})
	MESSAGE(STATUS "Boost_INCLUDES: " ${Boost_INCLUDES})
	MESSAGE(STATUS "Boost_LIBRARIES: " ${Boost_LIBRARIES})


#find_package(yaml-cpp REQUIRED)
find_library (yaml_LIB yaml-cpp)

add_subdirectory(server)

