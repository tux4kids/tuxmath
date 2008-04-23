project (tuxmath)

set (TUXMATH_VERSION "1.6.1")
set (TUXMATHADMIN_VERSION "0.1.1")

## Allow loading of any tuxmath-specific CMake modules
set(CMAKE_MODULE_PATH "${tuxmath_SOURCE_DIR}/cmake-modules")

if (UNIX AND NOT APPLE)
  message("CMAKE_INSTALL_PREFIX is ${CMAKE_INSTALL_PREFIX}")
endif (UNIX AND NOT APPLE)
message("TUXMATH_VERSION is ${TUXMATH_VERSION}")

## Define the extra paths
set (DATA_TUXMATH ${tuxmath_SOURCE_DIR}/data)
set (DOC_TUXMATH ${tuxmath_SOURCE_DIR}/doc)
set (PO_TUXMATH ${tuxmath_SOURCE_DIR}/po)

## Set up OS-specific path information
set (EXTRA_INCLUDE)  # makes it blank
set (EXTRA_SRC)      # makes it blank
if (APPLE)
  set (MACOSX_BUNDLE_NAME tuxmath)
  set(DATA_PREFIX ${MACOSX_BUNDLE_NAME}.app/Contents/Resources)
  set(LOCALEDIR ${MACOSX_BUNDLE_NAME}.app/Contents/Resources/locale)
  # OS X does not have a pre-built libSDLmain, so we instead include
  # SDLmain.m as a source file
  set (EXTRA_INCLUDE ${tuxmath_SOURCE_DIR}/macosx)
  set (EXTRA_SRC ${tuxmath_SOURCE_DIR}/macosx/SDLMain.m)
elseif (UNIX)
  # A non-apple Unix (Linux, Solaris, etc.)
  set (DATA_PREFIX ${CMAKE_INSTALL_PREFIX}/share/tuxmath)
  set (LOCALEDIR ${CMAKE_INSTALL_PREFIX}/share/locale)
endif (APPLE)

## Code for internationalization: if we don't have gettext, we need to
## compile the intl library.  In some cases, we will need to compile
## the intl library even if we have gettext---we might not be able to
## count on the installed system having it, even if the development
## system does.
include (CheckSymbolExists)
check_symbol_exists (gettext libintl.h HAVE_GETTEXT)
find_package (MSGFMT)
#message("MSGFMT_FOUND ${MSGFMT_FOUND}    MSGFMT_EXECUTABLE ${MSGFMT_EXECUTABLE}")
set (BUILD_INTL FALSE)
if (DEFINED HAVE_GETTEXT)
  set (BUILD_INTL TRUE)
endif (DEFINED HAVE_GETTEXT)
if (NOT MSGFMT_FOUND)
  message("Warning: internationalization disabled")
endif (NOT MSGFMT_FOUND)
message("HAVE_GETTEXT ${HAVE_GETTEXT}   BUILD_INTL ${BUILD_INTL}")
if (APPLE)
  set (BUILD_INTL TRUE)
endif (APPLE)
message("HAVE_GETTEXT ${HAVE_GETTEXT}   BUILD_INTL ${BUILD_INTL}")

## Add subdirectories
add_subdirectory (src)
add_subdirectory (data)
add_subdirectory (doc)
if (MSGFMT_FOUND)
  add_subdirectory (po)
endif (MSGFMT_FOUND)
if (BUILD_INTL)
  add_subdirectory (intl)
endif (BUILD_INTL)

if (APPLE)
  macro (copyFramework headerDir destDir)
    string(REPLACE "/Headers" "" FRAMEWORK_SOURCE_DIR ${headerDir})
    install(DIRECTORY ${FRAMEWORK_SOURCE_DIR} DESTINATION ${destDir})
  endmacro(copyFramework)
  set(FRAMEWORK_DEST_DIR ${tuxmath_BINARY_DIR}/src/${MACOSX_BUNDLE_NAME}.app/Contents/Frameworks)
  copyFramework(${SDL_INCLUDE_DIR} ${FRAMEWORK_DEST_DIR})
  copyFramework(${SDLMIXER_INCLUDE_DIR} ${FRAMEWORK_DEST_DIR})
  copyFramework(${SDLIMAGE_INCLUDE_DIR} ${FRAMEWORK_DEST_DIR})
  copyFramework(${SDLTTF_INCLUDE_DIR} ${FRAMEWORK_DEST_DIR})
endif (APPLE)
