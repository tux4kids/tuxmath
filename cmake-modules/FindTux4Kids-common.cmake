# - Locate Tux4Kids-common library
# This module defines
#  T4KCOMMON_LIBRARY, the library to link against
#  T4KCOMMON_FOUND, if false, do not try to link to t4kcommon
#  T4KCOMMON_INCLUDE_DIR, where to find t4k_common.h
#   
# Adapted from FindSDL_Pango.cmake, which was:
# Created by Tim Holy. This was influenced by the FindSDL_ttf.cmake
# module by Eric Wing.
find_path(T4KCOMMON_INCLUDE_DIR t4k_common.h
  $ENV{T4KCOMMONDIR}
  $ENV{T4KCOMMONDIR}/include
  ~/Library/Frameworks/Tux4Kids-common.framework/Headers
  /Library/Frameworks/Tux4Kids-common.framework/Headers
  /usr/local/include/t4k_common
  /usr/include/t4k_common
  /usr/local/include
  /usr/include
  /sw/include/t4k_common # Fink
  /sw/include
  /opt/local/include/t4k_common # DarwinPorts
  /opt/local/include
  /opt/csw/include/t4k_common # Blastwave
  /opt/csw/include 
  /opt/include/t4k_common
  /opt/include
  )
# I'm not sure if I should do a special casing for Apple. It is 
# unlikely that other Unix systems will find the framework path.
# But if they do ([Next|Open|GNU]Step?), 
# do they want the -framework option also?
if(${T4KCOMMON_INCLUDE_DIR} MATCHES ".framework")
  # Extract the path the framework resides in so we can use it for the -F flag
  string(REGEX REPLACE "(.*)/.*\\.framework/.*" "\\1"
	T4KCOMMON_FRAMEWORK_PATH_TEMP ${T4KCOMMON_INCLUDE_DIR})
  if("${T4KCOMMON_FRAMEWORK_PATH_TEMP}" STREQUAL "/Library/Frameworks"
      OR "${T4KCOMMON_FRAMEWORK_PATH_TEMP}" STREQUAL "/System/Library/Frameworks"
      )
    # String is in default search path, don't need to use -F
    set(T4KCOMMON_LIBRARY "-framework Tux4Kids-common" CACHE STRING
    "Tux4Kids-common framework for OSX")
  else("${T4KCOMMON_FRAMEWORK_PATH_TEMP}" STREQUAL "/Library/Frameworks"
      OR "${T4KCOMMON_FRAMEWORK_PATH_TEMP}" STREQUAL "/System/Library/Frameworks"
      )
    # String is not /Library/Frameworks, need to use -F
    set(T4KCOMMON_LIBRARY "-F${T4KCOMMON_FRAMEWORK_PATH_TEMP} -framework
    Tux4Kids-common" CACHE STRING "Tux4Kids-common framework for OSX")
  endif("${T4KCOMMON_FRAMEWORK_PATH_TEMP}" STREQUAL "/Library/Frameworks"
    OR "${T4KCOMMON_FRAMEWORK_PATH_TEMP}" STREQUAL "/System/Library/Frameworks"
    )
  # Clear the temp variable so nobody can see it
  set(T4KCOMMON_FRAMEWORK_PATH_TEMP "" CACHE INTERNAL "")

else(${T4KCOMMON_INCLUDE_DIR} MATCHES ".framework")
  find_library(T4KCOMMON_LIBRARY 
    NAMES t4k_common
    PATHS
    $ENV{T4KCOMMONDIR}/lib
    /usr/local/lib
    /usr/lib
    /sw/lib
    /opt/local/lib
    /opt/csw/lib
    /opt/lib
    )
endif(${T4KCOMMON_INCLUDE_DIR} MATCHES ".framework")

if(T4KCOMMON_LIBRARY)
  set(T4KCOMMON_FOUND "YES")
else()
  set(T4KCOMMON_FOUND "NO")
  set(FINDSTR "NOT ")
endif()

message("Looking for libt4k_common...${FINDSTR}found!")

