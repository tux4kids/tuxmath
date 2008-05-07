#The following isn't necessary because of the SDL workaround
#cmake_minimum_required(VERSION 2.4.7 FATAL_ERROR)

## Define the source files used for each executable
# tuxmath
set(SOURCES_TUXMATH
  audio.c
  ConvertUTF.c
  credits.c
  fileops.c
  game.c
  highscore.c
  lessons.c
  loaders.c
  mathcards.c
  options.c
  scandir.c
  SDL_extras.c
  setup.c
  titlescreen.c
  tuxmath.c
  ${TUXMATH_EXTRA_SRC}
  )

message("${SOURCES_TUXMATH}")

# tuxmathadmin
set(SOURCES_TUXMATHADMIN
  tuxmathadmin.c
  )

## Libraries
find_package(SDL REQUIRED)
find_package(SDL_image REQUIRED)
find_package(SDL_ttf REQUIRED)
find_package(SDL_mixer REQUIRED)

if (NOT SDL_FOUND)
  # Workaround for REQUIRED flag not working with cmake < 2.4.7.
  # Should put other libraries in, too.
  message(FATAL_ERROR "SDL not found!")
endif (NOT SDL_FOUND)

if (NOT APPLE)
  link_libraries (SDLmain)
endif (NOT APPLE)

## Include files
#if (NOT ENABLE_NLS)
   # If you want internationalization, this doesn't work: you have to
   # run automake's "configure" and then copy config.h into the build
   # directory
   configure_file(${TuxMath_SOURCE_DIR}/config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)
#endif (NOT ENABLE_NLS)
#check_include_file(config.h HAVE_CONFIG_H REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR} ${SDL_INCLUDE_DIR} ${SDLIMAGE_INCLUDE_DIR} ${SDLMIXER_INCLUDE_DIR} ${SDLTTF_INCLUDE_DIR} ${TUXMATH_EXTRA_INCLUDES} ${INTL_BINARY_DIR})

if (TUXMATH_BUILD_INTL)
  link_directories(${INTL_BINARY_DIR})
  message("Added link directory ${INTL_BINARY_DIR}")
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

set_target_properties (
  tuxmath
  PROPERTIES COMPILE_FLAGS 
  "-DDATA_PREFIX=\\\"${TUXMATH_DATA_PREFIX}\\\" -DVERSION=\\\"${TUXMATH_VERSION}\\\" -DLOCALEDIR=\\\"${LOCALE_DIR}\\\" -DPACKAGE=\\\"tuxmath\\\""
  )

target_link_libraries (tuxmath
  ${SDL_LIBRARY}
  ${SDLIMAGE_LIBRARY}
  ${SDLTTF_LIBRARY}
  ${SDLMIXER_LIBRARY}
  )

if (APPLE)
  # The following seems to be needed to compile under 10.5
  set_target_properties(tuxmath tuxmathadmin
    PROPERTIES
    LINK_FLAGS "-mmacosx-version-min=10.4")
endif(APPLE)

if (TUXMATH_BUILD_INTL)
  message("Linking iconv: ${ICONV_LIBRARIES}")
  target_link_libraries(tuxmath ${ICONV_LIBRARIES} libintl.a)
endif(TUXMATH_BUILD_INTL)

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
