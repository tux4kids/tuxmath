# This file implements a build for TuxMath using CMake
# License information can be found in doc/COPYING.txt
#
# Copyright 2008-2011 by Timothy E. Holy
#
# This was based on many examples, but a particular debt is owed to
# Peter K�mmel of the LyX project.

# NOTE - the CMake build basically works, but is not completely 
# up-to-date and does not check for as many libs, headers,
# programs, and functions.  The Autotools build is the basis
# for official releases and is more consistently maintained.

project(TuxMath)


set(TUXMATH_VERSION "2.0.4")
set(TUXMATHADMIN_VERSION "0.1.1")
message("Building TuxMath version ${TUXMATH_VERSION}")

## Setting up CMake itself
cmake_minimum_required(VERSION 2.6)
set(CMAKE_MODULE_PATH "${TuxMath_SOURCE_DIR}/cmake-modules")
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)

#message("CMP: ${CMAKE_MODULE_PATH}")

include(T4K_Sugar)

## Choose a debug build set
t4k_gentle_set(CMAKE_BUILD_TYPE Debug)

## Decide on our build-type: installed or relocatable
## Linux (& Windows?) would be installed, Mac relocatable
## Someday we might supply a Linux relocatable package; this would be
## specified by executing cmake -DTUXMATH_BUILD_TYPE=relocatable
if (APPLE)
  set(TUXMATH_BUILD_TYPE relocatable CACHE STRING "Installed or relocatable")
else (APPLE)
  set(TUXMATH_BUILD_TYPE installed CACHE STRING "Installed or relocatable")
endif (APPLE)

message("TBT IS ${TUXMATH_BUILD_TYPE}")

if (TUXMATH_BUILD_TYPE MATCHES installed)
  message("The installation location is ${CMAKE_INSTALL_PREFIX}")
endif (TUXMATH_BUILD_TYPE MATCHES installed)

## Define the extra paths
#set (DATA_TUXMATH ${tuxmath_SOURCE_DIR}/data)
#set (DOC_TUXMATH ${tuxmath_SOURCE_DIR}/doc)
#set (PO_TUXMATH ${tuxmath_SOURCE_DIR}/po)

## Set up OS-specific path & build information
if (APPLE)
  set(TUXMATH_MACOSX_BUNDLE_NAME tuxmath)
  set(MACOSX_BUNDLE_BUNDLE_VERSION ${TUXMATH_VERSION})
  set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${TUXMATH_VERSION})
  set(MACOSX_BUNDLE_LONG_VERSION_STRING ${TUXMATH_VERSION})
  set(MACOSX_BUNDLE_ICON_FILE tuxmath.icns)
  set(TUXMATH_DATA_PREFIX ${TUXMATH_MACOSX_BUNDLE_NAME}.app/Contents/Resources)
  set(LOCALE_DIR ${TUXMATH_MACOSX_BUNDLE_NAME}.app/Contents/Resources/locale)
  set (TUXMATH_EXTRA_INCLUDE ${TuxMath_SOURCE_DIR}/macosx)
  # OS X SDL Framework does not have a pre-built libSDLmain, so we
  # instead include SDLmain.m as a source file
  set (TUXMATH_EXTRA_SRC ${TuxMath_SOURCE_DIR}/macosx/SDLMain.m)
  # Build universal binaries 
  set(CMAKE_OSX_ARCHITECTURES ppc i386)    
elseif (UNIX)
  # A non-apple Unix (Linux, Solaris, etc.)
  # This is where one would test for installed/relocatable, if that
  # choice is implemented.  For now we just provide installed.
  set (TUXMATH_DATA_PREFIX ${CMAKE_INSTALL_PREFIX}/share/tuxmath)
  set (LOCALE_DIR ${CMAKE_INSTALL_PREFIX}/share/locale)
elseif (MINGW)
  set (BUILD_MINGW32 TRUE)
  t4k_include_definition(BUILD_MINGW32)
  add_definitions(-DPROGRAM_NAME="TuxMath")
  if (CMAKE_BUILD_TYPE MATCHES Debug)
    set (TUXMATH_DATA_PREFIX "${CMAKE_SOURCE_DIR}/data" CACHE STRING "Data files")
  elseif (TUXMATH_BUILD_TYPE MATCHES relocatable)
    set (TUXMATH_DATA_PREFIX "./data" CACHE STRING "Data files")
  else() # installed
    string( REPLACE "\\" "/" TUXMATH_DATA_PREFIX "$ENV{USERPROFILE}/Tux4Kids")
  endif()
endif (APPLE)
#message("TDP ${TUXMATH_DATA_PREFIX}")


## Decide on whether we're going to build with internationalization
# We can't build with internationalization unless the build computer
# supports msgfmt
find_package(MSGFMT)
if (NOT DEFINED ENABLE_NLS OR NOT MSGFMT_FOUND)
  # You can disable NLS with -DENABLE_NLS=false
  set(ENABLE_NLS ${MSGFMT_FOUND})
endif (NOT DEFINED ENABLE_NLS OR NOT MSGFMT_FOUND)
if (NOT ENABLE_NLS)
  message("Internationalization disabled")
endif (NOT ENABLE_NLS)
# Even if the build computer supports gettext/msgfmt, we may not be
# able to guarantee that the computers on which it is installed provides
# runtime support for gettext.  So we may have to build gettext.
if (ENABLE_NLS)
  if (APPLE)
    set(TUXMATH_BUILD_INTL true)
  endif (APPLE)
else (ENABLE_NLS)
  set(TUXMATH_BUILD_INTL false)  # No point in building intl if no NLS
endif (ENABLE_NLS)
message("ENABLE_NLS ${ENABLE_NLS}")
set(HAVE_GETTEXT ENABLE_NLS)
# Check to see if we have SDL_Pango support for the right-to-left languages
if (ENABLE_NLS)
  find_package(SDL_Pango)
  set(SDL_Pango ${SDLPANGO_FOUND})  # For the config.h file
endif (ENABLE_NLS)


## Don't think we need SDL_gfx anymore:
##FIND_PACKAGE(SDL_gfx REQUIRED)

# Check for libRSVG for SVG support

include(FindPkgConfig OPTIONAL)

if(PKG_CONFIG_FOUND)
  include(FindPkgConfig)
  PKG_CHECK_MODULES(RSVG librsvg-2.0)
  if(RSVG_FOUND)
    PKG_CHECK_MODULES(CAIRO cairo)
    if(CAIRO_FOUND)
      set(HAVE_RSVG 1)  # For the config.h file
    endif(CAIRO_FOUND)
  endif(RSVG_FOUND)
endif(PKG_CONFIG_FOUND)


## Run configure checks
if (TUXMATH_BUILD_INTL)
  include(ConfigureChecksIntl)
endif (TUXMATH_BUILD_INTL)
include(ConfigureChecks)

## Add subdirectories
if (TUXMATH_BUILD_INTL)
  set(INTL_BINARY_DIR ${CMAKE_BINARY_DIR}/intl)
  set(INTL_SOURCE_DIR ${CMAKE_SOURCE_DIR}/intl)
  set(TOP_SRC_DIR ${CMAKE_BINARY_DIR})
  set(PREFIX ${CMAKE_BINARY_DIR})
  add_subdirectory(intl)
endif (TUXMATH_BUILD_INTL)
add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(doc)
if (ENABLE_NLS)
  set(LOCALE_INSTALL_DIR ${CMAKE_BINARY_DIR}/src/${LOCALE_DIR})
  add_subdirectory(po)
endif (ENABLE_NLS)

message("TMBD ${TuxMath_BINARY_DIR}")
#message("TMBN ${TUXMATH_MACOSX_BUNDLE_NAME}")

if (APPLE)
   add_subdirectory(macosx)
endif (APPLE)

