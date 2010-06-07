# Build script for the "linebreak" library, which provides the
#"linebreak" utilities from libgettextpo
#
# Timothy E. Holy, 2009

# If you're building this as part of a larger package, the following
# variables can be set upon entry:
#   LINEBREAK_BINARY_DIR
#   LINEBREAK_SOURCE_DIR
#   LOCALE_DIR
#   TOP_SRC_DIR
#   PREFIX
project(linebreak)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true)
include(CheckFunctionExists)
include(T4K_Sugar)
# If not found, we use SDL_iconv instead
find_package(Iconv)
#message("Iconv const: ${ICONV_SECOND_ARGUMENT_IS_CONST}")
set(HAVE_ICONV ${ICONV_FOUND})
t4k_include_definition(HAVE_ICONV)
# This can either be called in standalone mode, or as part of a larger
# project.  For that reason, we need to examine whether certain settings
# have been made before, or not.

# Removed the conditional since ConfigureChecksIntl 
# guards itself with INTL_CHECKS_DONE, is that okay? -BML
include(ConfigureChecksIntl)
t4k_gentle_set(LINEBREAK_BINARY_DIR ${CMAKE_BINARY_DIR})
t4k_gentle_set(LINEBREAK_SOURCE_DIR ${CMAKE_SOURCE_DIR})

# Generate the appropriate config.h file
configure_file(linebreak_config.h.cmake ${LINEBREAK_BINARY_DIR}/config.h)
add_definitions(
    -DHAVE_CONFIG_H=1
    -DMAKE_LINEBREAK_LIB
    -DIN_LIBLINEBREAK
    -DENABLE_RELOCATABLE=1
    -DIN_LIBRARY
    -DNO_XMALLOC
    -Dset_relocation_prefix=liblinebreak_set_relocation_prefix
    -Drelocate=liblinebreak_relocate
    -DDEPENDS_ON_LIBICONV=1
    )
    
if (APPLE)
set_directory_properties(PROPERTIES COMPILE_DEFINITIONS
	LOCALEDIR='\"${LOCALE_DIR}\"'        	
	LOCALE_ALIAS_PATH='\"${LOCALE_DIR}\"'
	LIBDIR='\"${TOP_SRC_DIR}\"'
	INSTALLDIR='\"${PREFIX}\"' 
	)
add_definitions(
		-DLOCALEDIR='\"${LOCALE_DIR}\"' 
		-DLOCALE_ALIAS_PATH='\"${LOCALE_DIR}\"'
		-DLIBDIR='\"${TOP_SRC_DIR}\"'
		-DINSTALLDIR='\"${PREFIX}\"' 
		)
else (APPLE)
add_definitions(
		-DLOCALEDIR=\\"${LOCALE_DIR}\\" 
		-DLOCALE_ALIAS_PATH=\\"${LOCALE_DIR}\\"
		-DLIBDIR=\\"${TOP_SRC_DIR}\\"
		-DINSTALLDIR=\\"${PREFIX}\\" 
		)
endif (APPLE)

file(GLOB_RECURSE LINEBREAK_HEADERS ${LINEBREAK_SOURCE_DIR} *.h)
file(GLOB_RECURSE LINEBREAK_SOURCES ${LINEBREAK_SOURCE_DIR} *.c)

include_directories(
  ${LINEBREAK_BINARY_DIR} 
  ${LINEBREAK_SOURCE_DIR} 
  ${CMAKE_CURRENT_BINARY_DIR})
  
if (ICONV_FOUND)
  include_directories(${ICONV_INCLUDE_DIR})
  target_link_libraries(linebreak ${ICONV_LIBRARY})
endif (ICONV_FOUND)
add_library(linebreak STATIC ${LINEBREAK_SOURCES} ${LINEBREAK_HEADERS})

#project_source_group("${GROUP_CODE}" LINEBREAK_SOURCES LINEBREAK_HEADERS)

