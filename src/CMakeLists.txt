#The following isn't necessary because of the SDL workaround
#cmake_minimum_required(VERSION 2.4.7 FATAL_ERROR)

## Libraries
find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED)
find_package(SDL_mixer REQUIRED)
find_package(SDL_gfx)
find_package(SDL_net)
find_package(Tux4Kids-common)

if (NOT SDLGFX_FOUND)
  message("Adding rotozoom")
  set(TUXMATH_EXTRA_SRC ${TUXMATH_EXTRA_SRC} SDL_rotozoom.c)
endif (NOT SDLGFX_FOUND)

if (T4KCOMMON_FOUND)
  set(HAVE_LIBT4K_COMMON 1)
endif(T4KCOMMON_FOUND)

if (SDLNET_FOUND)
  set(HAVE_LIBSDL_NET 1)
  list(APPEND TUXMATH_EXTRA_SRC server.c network.c)
endif (SDLNET_FOUND)

## Define the source files used for each executable
# tuxmath
set(SOURCES_TUXMATH
  audio.c
  comets.c
  comets_graphics.c
  credits.c
  draw_utils.c
  factoroids.c
  factoroids_graphics.c
  fileops.c
  fileops_media.c
  frame_counter.c
  game.c
  menu.c
  menu_lan.c
  highscore.c
  lessons.c
  mathcards.c
  options.c
  setup.c
  titlescreen.c
  multiplayer.c
  campaign.c
  tuxmath.c
  ${TUXMATH_EXTRA_SRC}
  )

message("${SOURCES_TUXMATH}")

# tuxmathadmin
set(SOURCES_TUXMATHADMIN
  tuxmathadmin.c
  )

if (NOT SDL_FOUND)
  # Workaround for REQUIRED flag not working with cmake < 2.4.7.
  # Should put other libraries in, too.
  message(FATAL_ERROR "SDL not found!")
endif (NOT SDL_FOUND)

## Include files
if (NOT BUILD_INTL)
  # Generate the config.h file
  configure_file(${TuxMath_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
endif (NOT BUILD_INTL)
check_include_file(${CMAKE_CURRENT_BINARY_DIR}/config.h HAVE_CONFIG_H)

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR} 
  ${SDL_INCLUDE_DIR} 
  ${SDLIMAGE_INCLUDE_DIR} 
  ${SDLMIXER_INCLUDE_DIR} 
  ${SDLNET_INCLUDE_DIR} 
  ${T4KCOMMON_INCLUDE_DIR}
  ${TUXMATH_EXTRA_INCLUDES} 
  ${INTL_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src
  )
if (SDLGFX_FOUND)  
  include_directories(${SDLGFX_INCLUDE_DIR})
endif(SDLGFX_FOUND)

if (TUXMATH_BUILD_INTL)
  link_directories(${INTL_BINARY_DIR})
endif(TUXMATH_BUILD_INTL)


## Define the executables
add_executable (
  tuxmath
  MACOSX_BUNDLE
  WIN32
  ${SOURCES_TUXMATH}
  )

add_executable (
  tuxmathadmin
  ${SOURCES_TUXMATHADMIN}
  )

# getting rid of semicolons
set(_rsvg_cflags "")
foreach(f ${RSVG_CFLAGS})
  set(_rsvg_cflags "${_rsvg_cflags} ${f}")
endforeach(f)

set(_cairo_cflags "")
foreach(f ${CAIRO_CFLAGS})
  set(_cairo_cflags "${_cairo_cflags} ${f}")
endforeach(f)

set(_rsvg_ldflags "")
foreach(f ${RSVG_LDFLAGS})
  set(_rsvg_ldflags "${_rsvg_ldflags} ${f}")
endforeach(f)

set(_cairo_ldflags "")
foreach(f ${CAIRO_LDFLAGS})
  set(_cairo_ldflags "${_cairo_ldflags} ${f}")
endforeach(f)

set_target_properties (
  tuxmath
  PROPERTIES COMPILE_FLAGS
  "-DDATA_PREFIX=\\\"${TUXMATH_DATA_PREFIX}\\\" -DVERSION=\\\"${TUXMATH_VERSION}\\\" -DLOCALEDIR=\\\"${LOCALE_DIR}\\\" -DPACKAGE=\\\"tuxmath\\\" ${_rsvg_cflags} ${_cairo_cflags}"
  LINK_FLAGS "${_rsvg_ldflags} ${_cairo_ldflags}"
  )

target_link_libraries (tuxmath
  ${SDL_LIBRARY}
  ${SDLIMAGE_LIBRARY}
  ${SDLMIXER_LIBRARY}
  ${SDLNET_LIBRARY}
  )

if (SDLGFX_FOUND)
   target_link_libraries (tuxmath
   ${SDLGFX_LIBRARY}
   )
endif (SDLGFX_FOUND)

if (T4KCOMMON_FOUND)
  target_link_libraries(tuxmath ${T4KCOMMON_LIBRARY})
endif ()

if (APPLE)
  # The following seems to be needed to compile under 10.5
  set_target_properties(tuxmath tuxmathadmin
    PROPERTIES
    LINK_FLAGS "-mmacosx-version-min=10.4")
endif(APPLE)

if (ICONV_FOUND)
  set(ICONV_TEMP ${ICONV_LIBRARIES})
  message("Linking iconv: ${ICONV_TEMP}")
endif()
if (TUXMATH_BUILD_INTL)
  target_link_libraries(tuxmath ${ICONV_TEMP} libintl.a)
endif(TUXMATH_BUILD_INTL)

#Some versions of binutils may have problems linking libm without an explicit declaration
if(UNIX AND NOT APPLE)
  target_link_libraries(tuxmath m)
endif(UNIX AND NOT APPLE)


set_target_properties (
  tuxmathadmin
  PROPERTIES COMPILE_FLAGS 
  "-DDATA_PREFIX=\\\"${TUXMATH_DATA_PREFIX}\\\" -DVERSION=\\\"${TUXMATHADMIN_VERSION}\\\" -DLOCALEDIR=\\\"${LOCALE_DIR}\\\" -DPACKAGE=\\\"tuxmathadmin\\\""
  )

## Installation specifications
if (UNIX AND NOT APPLE)
  install (TARGETS tuxmath tuxmathadmin
    RUNTIME DESTINATION bin)
endif(UNIX AND NOT APPLE)
