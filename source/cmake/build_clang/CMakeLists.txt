CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(MODE release)

SET(COMPILER clang)
SET(CMAKE_C_COMPILER clang)
SET(CMAKE_CXX_COMPILER clang++)
#SET(COMPILER gcc)
SET(LINK_RULES shared)
#SET(LINK_RULES static)

INCLUDE(../build/build.pre.cmake)
INCLUDE(../build/${LINK_RULES}.build.cmake)

ADD_DEFINITIONS(-DNDEBUG)

SET(COMPILE_FLAGS "-pipe -w -O3 -std=c++11 -Qunused-arguments -DUNUSUAL_ALLOCATOR_DECLARATION -ftemplate-depth-256 -stdlib=libstdc++")
SET(CMAKE_CXX_FLAGS "${COMPILE_FLAGS}")

INCLUDE(../build/build.post.cmake)

## Top level binaries
INCLUDE( ../build/apps.all.cmake )
INCLUDE( ../build/pilot_apps.all.cmake )
