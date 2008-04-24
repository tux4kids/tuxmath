# This file implements a build for TuxMath using CMake
# License information can be found in doc/COPYING.txt
#
# Copyright 2008 by Timothy E. Holy
#
# This was based on many examples, but a particular debt is owed to
# Peter Kümmel of the LyX project.

project(TuxMath)

set(TUXMATH_VERSION "1.6.1")
set(TUXMATHADMIN_VERSION "0.1.1")
message("Building TuxMath version ${TUXMATH_VERSION}")

## Setting up CMake itself
cmake_minimum_required(VERSION 2.4)
set(CMAKE_MODULE_PATH "${TuxMath_SOURCE_DIR}/cmake-modules")
message("CMP: ${CMAKE_MODULE_PATH}")

## Decide on our build-type: installed or relocatable
## Linux (& Windows?) would be installed, Mac relocatable
## Someday we might supply a Linux relocatable package; this would be
## specified by executing cmake -DTUXMATH_BUILD_TYPE=relocatable
if (NOT TUXMATH_BUILD_TYPE)
  if (APPLE)
    set(TUXMATH_BUILD_TYPE relocatable)
  else (APPLE)
    set(TUXMATH_BUILD_TYPE installed)
  endif (APPLE)
endif (NOT TUXMATH_BUILD_TYPE)
if (TUXMATH_BUILD_TYPE MATCHES installed)
  message("The installation location is ${CMAKE_INSTALL_PREFIX}")
endif (TUXMATH_BUILD_TYPE MATCHES installed)

## Define the extra paths
#set (DATA_TUXMATH ${tuxmath_SOURCE_DIR}/data)
#set (DOC_TUXMATH ${tuxmath_SOURCE_DIR}/doc)
#set (PO_TUXMATH ${tuxmath_SOURCE_DIR}/po)

## Set up OS-specific path & build information
if (APPLE)
  set(TUXMATH_MACOSX_BUNDLE_NAME ${PROJECT_NAME})
  set(TUXMATH_DATA_PREFIX ${TUXMATH_MACOSX_BUNDLE_NAME}.app/Contents/Resources)
  set(LOCALE_DIR ${TUXMATH_MACOSX_BUNDLE_NAME}.app/Contents/Resources/locale)
  # OS X SDL Framework does not have a pre-built libSDLmain, so we
  # instead include SDLmain.m as a source file
  set (TUXMATH_EXTRA_INCLUDE ${tuxmath_SOURCE_DIR}/macosx)
  set (TUXMATH_EXTRA_SRC ${tuxmath_SOURCE_DIR}/macosx/SDLMain.m)
elseif (UNIX)
  # A non-apple Unix (Linux, Solaris, etc.)
  # This is where one would test for installed/relocatable, if that
  # choice is implemented.  For now we just provide installed.
  set (TUXMATH_DATA_PREFIX ${CMAKE_INSTALL_PREFIX}/share/tuxmath)
  set (LOCALE_DIR ${CMAKE_INSTALL_PREFIX}/share/locale)
endif (APPLE)

## Decide on whether we're going to build with internationalization
# We can't build with internationalization unless the build computer
# supports msgfmt
find_package(MSGFMT)
if (NOT DEFINED TUXMATH_NLS OR NOT MSGFMT_FOUND)
  # You can disable NLS with -DTUXMATH_NLS=false
  set(TUXMATH_NLS ${MSGFMT_FOUND})
endif (NOT DEFINED TUXMATH_NLS OR NOT MSGFMT_FOUND)
if (NOT TUXMATH_NLS)
  message("Internationalization disabled")
endif (NOT TUXMATH_NLS)
# Even if the build computer supports gettext/msgfmt, we may not be
# able to guarantee that the computers on which it is installed provides
# runtime support for gettext.  So we may have to build gettext.
if (TUXMATH_NLS)
  if (APPLE)
    set(TUXMATH_BUILD_INTL true)
  endif (APPLE)
else (TUXMATH_NLS)
  set(TUXMATH_BUILD_INTL false)  # No point in building intl if no NLS
endif (TUXMATH_NLS)


## Run configure checks
if (TUXMATH_BUILD_INTL)
  include(ConfigureChecksIntl)
endif (TUXMATH_BUILD_INTL)
include(ConfigureChecks)

## Add subdirectories
add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(doc)
if (TUXMATH_NLS)
  add_subdirectory(po)
endif (TUXMATH_NLS)
if (TUXMATH_BUILD_INTL)
  set(INTL_BINARY_DIR ${CMAKE_BINARY_DIR}/intl)
  set(INTL_SOURCE_DIR ${CMAKE_SOURCE_DIR}/intl)
  set(TOP_SRC_DIR ${CMAKE_BINARY_DIR})
  set(PREFIX ${CMAKE_BINARY_DIR})
  add_subdirectory(intl)
endif (TUXMATH_BUILD_INTL)

## Copy any needed libraries to the installation target
if (APPLE)
  macro (copyFramework headerDir destDir)
    string(REPLACE "/Headers" "" FRAMEWORK_SOURCE_DIR ${headerDir})
    install(DIRECTORY ${FRAMEWORK_SOURCE_DIR} DESTINATION ${destDir})
  endmacro(copyFramework)
  set(FRAMEWORK_DEST_DIR ${tuxmath_BINARY_DIR}/src/${TUXMATH_MACOSX_BUNDLE_NAME}.app/Contents/Frameworks)
  copyFramework(${SDL_INCLUDE_DIR} ${FRAMEWORK_DEST_DIR})
  copyFramework(${SDLMIXER_INCLUDE_DIR} ${FRAMEWORK_DEST_DIR})
  copyFramework(${SDLIMAGE_INCLUDE_DIR} ${FRAMEWORK_DEST_DIR})
  copyFramework(${SDLTTF_INCLUDE_DIR} ${FRAMEWORK_DEST_DIR})
endif (APPLE)
