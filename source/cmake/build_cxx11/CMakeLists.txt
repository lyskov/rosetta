CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

##############################################################################
# This file only needs to specify which mode (i.e., release, debug),
# extras (i.e., graphics), and compiler (i.e., gcc, clang) should be used. The
# appropriate settings for each mode, extras, and compiler should be
# implemented in ../build/build.settings.cmake.
##############################################################################

project (Rosetta)

############ Build Type, Mode, Link Rules, Compiler, Extras
SET(EXTRAS cxx11)
SET(MODE release)
#SET(COMPILER gcc)
SET(LINK_RULES shared)

############ Build Rules
INCLUDE(../build/compiler.cmake)
INCLUDE(../build/build.pre.cmake)
INCLUDE(../build/${LINK_RULES}.build.cmake)
INCLUDE(../build/build.settings.cmake)
INCLUDE(../build/build.post.cmake)

############ Top level binaries
INCLUDE( ../build/apps.all.cmake )
INCLUDE( ../build/pilot_apps.all.cmake )


REMOVE_DEFINITIONS(-DPTR_MODERN)
REMOVE_DEFINITIONS(-DPTR_BOOST)

ADD_DEFINITIONS(-DCXX11)
ADD_DEFINITIONS(-DPTR_STD)
