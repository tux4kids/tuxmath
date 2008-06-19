# - Locate SDL_Pango library
# This module defines
#  SDLPANGO_LIBRARY, the library to link against
#  SDLPANGO_FOUND, if false, do not try to link to SDL
#  SDLPANGO_INCLUDE_DIR, where to find SDL/SDL.h
#   
# $SDLDIR is an environment variable that would
# correspond to the ./configure --prefix=$SDLDIR
# used in building SDL.
# Created by Tim Holy. This was influenced by the FindSDL_ttf.cmake
# module by Eric Wing.
# An SDL_Pango framework doesn't seem to exist for OS X, so the rest
# of the comments below are probably not relevant.
# This has modifications to recognize OS X frameworks and 
# additional Unix paths (FreeBSD, etc).
# On OSX, this will prefer the Framework version (if found) over others.
# People will have to manually change the cache values of 
# SDLPANGO_LIBRARY to override this selection.
FIND_PATH(SDLPANGO_INCLUDE_DIR SDL_Pango.h
  $ENV{SDLPANGODIR}/include
  $ENV{SDLDIR}/include
  ~/Library/Frameworks/SDL_Pango.framework/Headers
  /Library/Frameworks/SDL_Pango.framework/Headers
  /usr/local/include/SDL
  /usr/include/SDL
  /usr/local/include/SDL12
  /usr/local/include/SDL11 # FreeBSD ports
  /usr/include/SDL12
  /usr/include/SDL11
  /usr/local/include
  /usr/include
  /sw/include/SDL # Fink
  /sw/include
  /opt/local/include/SDL # DarwinPorts
  /opt/local/include
  /opt/csw/include/SDL # Blastwave
  /opt/csw/include 
  /opt/include/SDL
  /opt/include
  )
# I'm not sure if I should do a special casing for Apple. It is 
# unlikely that other Unix systems will find the framework path.
# But if they do ([Next|Open|GNU]Step?), 
# do they want the -framework option also?
IF(${SDLPANGO_INCLUDE_DIR} MATCHES ".framework")
  # Extract the path the framework resides in so we can use it for the -F flag
  STRING(REGEX REPLACE "(.*)/.*\\.framework/.*" "\\1" SDLPANGO_FRAMEWORK_PATH_TEMP ${SDLPANGO_INCLUDE_DIR})
  IF("${SDLPANGO_FRAMEWORK_PATH_TEMP}" STREQUAL "/Library/Frameworks"
      OR "${SDLPANGO_FRAMEWORK_PATH_TEMP}" STREQUAL "/System/Library/Frameworks"
      )
    # String is in default search path, don't need to use -F
    SET(SDLPANGO_LIBRARY "-framework SDL_Pango" CACHE STRING "SDL_Pango framework for OSX")
  ELSE("${SDLPANGO_FRAMEWORK_PATH_TEMP}" STREQUAL "/Library/Frameworks"
      OR "${SDLPANGO_FRAMEWORK_PATH_TEMP}" STREQUAL "/System/Library/Frameworks"
      )
    # String is not /Library/Frameworks, need to use -F
    SET(SDLPANGO_LIBRARY "-F${SDLPANGO_FRAMEWORK_PATH_TEMP} -framework SDL_Pango" CACHE STRING "SDL_Pango framework for OSX")
  ENDIF("${SDLPANGO_FRAMEWORK_PATH_TEMP}" STREQUAL "/Library/Frameworks"
    OR "${SDLPANGO_FRAMEWORK_PATH_TEMP}" STREQUAL "/System/Library/Frameworks"
    )
  # Clear the temp variable so nobody can see it
  SET(SDLPANGO_FRAMEWORK_PATH_TEMP "" CACHE INTERNAL "")

ELSE(${SDLPANGO_INCLUDE_DIR} MATCHES ".framework")
  FIND_LIBRARY(SDLPANGO_LIBRARY 
    NAMES SDL_Pango
    PATHS
    $ENV{SDLPANGODIR}/lib
    $ENV{SDLDIR}/lib
    /usr/local/lib
    /usr/lib
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    )
ENDIF(${SDLPANGO_INCLUDE_DIR} MATCHES ".framework")

SET(SDLPANGO_FOUND "NO")
IF(SDLPANGO_LIBRARY)
  SET(SDLPANGO_FOUND "YES")
ENDIF(SDLPANGO_LIBRARY)

