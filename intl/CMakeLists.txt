# Build script for libintl. This was copied, with only slight
# modifications, from the LyX project---thanks!  See attribution below.
#
# Modifications by Timothy E. Holy, 2008

# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

# If you're building this as part of a larger package, the following
# variables can be set upon entry:
#   INTL_BINARY_DIR
#   INTL_SOURCE_DIR
#   LOCALE_DIR
#   TOP_SRC_DIR  ?
#   PREFIX    ?

project(intl)

set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)
include(CheckFunctionExists)

# We require the iconv library to be available
find_package(Iconv REQUIRED)
#message("Iconv const: ${ICONV_SECOND_ARGUMENT_IS_CONST}")
add_definitions(-DHAVE_ICONV=1)

# This can either be called in standalone mode, or as part of a larger
# project.  For that reason, we need to examine whether certain settings
# have been made before, or not.
if (NOT CONFIGURECHECKSINTL_DONE)
  include(ConfigureChecksIntl)
endif ()
if (NOT INTL_BINARY_DIR)
  set(INTL_BINARY_DIR ${CMAKE_BINARY_DIR})
endif ()
if (NOT INTL_SOURCE_DIR)
  set(INTL_SOURCE_DIR ${CMAKE_SOURCE_DIR})
endif ()

# Generate the appropriate header files
configure_file(libgnuintl.h.in ${INTL_BINARY_DIR}/libgnuintl.h)
configure_file(${INTL_BINARY_DIR}/libgnuintl.h ${INTL_BINARY_DIR}/libintl.h COPYONLY)
# Generate the appropriate config.h file
configure_file(intl_config.h.cmake ${INTL_BINARY_DIR}/config.h)

add_definitions(
    -DHAVE_CONFIG_H=1
    -DMAKE_INTL_LIB
    -DIN_LIBINTL
    -DENABLE_RELOCATABLE=1
    -DIN_LIBRARY
    -DNO_XMALLOC
    -Dset_relocation_prefix=libintl_set_relocation_prefix
    -Drelocate=libintl_relocate
    -DDEPENDS_ON_LIBICONV=1
    )
    
if (NOT APPLE)    
add_definitions(
		-DLOCALEDIR=\\"${LOCALE_DIR}\\" 
		-DLOCALE_ALIAS_PATH=\\"${LOCALE_DIR}\\"
		-DLIBDIR=\\"${TOP_SRC_DIR}\\"
		-DINSTALLDIR=\\"${PREFIX}\\" 
		)
else (NOT APPLE) 
add_definitions(
		-DLOCALEDIR='\"${LOCALE_DIR}\"' 
		-DLOCALE_ALIAS_PATH='\"${LOCALE_DIR}\"'
		-DLIBDIR='\"${TOP_SRC_DIR}\"'
		-DINSTALLDIR='\"${PREFIX}\"' 
		)
endif (NOT APPLE)

file(GLOB INTL_HEADERS ${INTL_SOURCE_DIR}/*.h)

set(INTL_SOURCES 
      bindtextdom.c
      dcgettext.c
      dgettext.c
      gettext.c
      finddomain.c
      loadmsgcat.c
      localealias.c
      textdomain.c
      l10nflist.c
      explodename.c
      dcigettext.c
      dcngettext.c
      dngettext.c
      hash-string.c
      langprefs.c
      ngettext.c
      plural.c
      plural-exp.c
      localcharset.c
      relocatable.c
      localename.c
      log.c
      printf.c
      osdep.c
      intl-compat.c
)

include_directories(${INTL_BINARY_DIR} ${INTL_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

if(ICONV_FOUND)
  include_directories(${ICONV_INCLUDE_DIR})
endif(ICONV_FOUND)

add_library(intl STATIC ${INTL_SOURCES} ${INTL_HEADERS})

if(ICONV_FOUND)
  target_link_libraries(intl ${ICONV_LIBRARY})
endif(ICONV_FOUND)

#project_source_group("${GROUP_CODE}" INTL_SOURCES INTL_HEADERS)

