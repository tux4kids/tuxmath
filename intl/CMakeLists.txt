# Build script for libintl. This was copied, with only slight
# modifications, from the LyX project---thanks!

# This file is part of LyX, the document processor.
# Licence details can be found in the file COPYING.
#
# Copyright (c) 2006, Peter Kümmel, <syntheticpp@gmx.net>
#

project(intl) 

# libgnuintl.h.in => libintl.h
configure_file(libgnuintl.h ${CMAKE_CURRENT_BINARY_DIR}/libgnuintl.h COPYONLY)
configure_file(libgnuintl.h ${CMAKE_CURRENT_BINARY_DIR}/libintl.h COPYONLY)


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
		-DLOCALEDIR=\\"${LOACLE_DIR}\\" 
		-DLOCALE_ALIAS_PATH=\\"${LOACLE_DIR}\\"
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

file(GLOB intl_headers ${TOP_SRC_DIR}/intl/*.h)

set(intl_sources 
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

lyx_add_path(intl_sources ${TOP_SRC_DIR}/intl)

include_directories(${TOP_SRC_DIR}/intl ${CMAKE_CURRENT_BINARY_DIR})

if(ICONV_FOUND)
	include_directories(${ICONV_INCLUDE_DIR})
endif(ICONV_FOUND)

add_library(intl STATIC ${intl_sources} ${intl_headers})

if(ICONV_FOUND)
	target_link_libraries(intl ${ICONV_LIBRARY})
endif(ICONV_FOUND)

project_source_group("${GROUP_CODE}" intl_sources intl_headers)

