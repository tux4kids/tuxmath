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
if (NOT LINEBREAK_BINARY_DIR)
  set(LINEBREAK_BINARY_DIR ${CMAKE_BINARY_DIR})
endif ()
if (NOT LINEBREAK_SOURCE_DIR)
  set(LINEBREAK_SOURCE_DIR ${CMAKE_SOURCE_DIR})
endif ()

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

file(GLOB_RECURSE LINEBREAK_HEADERS ${LINEBREAK_SOURCE_DIR} *.h)

file(GLOB_RECURSE LINEBREAK_SOURCES ${LINEBREAK_SOURCE_DIR} *.c)


include_directories(${LINEBREAK_BINARY_DIR} ${LINEBREAK_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

if(ICONV_FOUND)
  include_directories(${ICONV_INCLUDE_DIR})
endif(ICONV_FOUND)

add_library(linebreak STATIC ${LINEBREAK_SOURCES} ${LINEBREAK_HEADERS})

if(ICONV_FOUND)
  target_link_libraries(linebreak ${ICONV_LIBRARY})
endif(ICONV_FOUND)

#project_source_group("${GROUP_CODE}" LINEBREAK_SOURCES LINEBREAK_HEADERS)

