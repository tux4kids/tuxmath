# The information in the data directory has to be placed in a location
# that tuxmath can find

message("Build type is ${TUXMATH_BUILD_TYPE}")

if (WIN32)
  if (TUXMATH_BUILD_TYPE MATCHES relocatable)
    set(DESTDIR "${CMAKE_INSTALL_PREFIX}/${TUXMATH_DATA_PREFIX}")
  else (TUXMATH_BUILD_TYPE MATCHES relocatable)
    set (DESTDIR ${TUXMATH_DATA_PREFIX})
  endif (TUXMATH_BUILD_TYPE MATCHES relocatable)
else(WIN32)
  if (APPLE)
    # The MACOSX_PACKAGE_LOCATION source file property doesn't seem to
    # work, so just do this by an explicit install command.
    set (DESTDIR "${TuxMath_BINARY_DIR}/src/${TUXMATH_DATA_PREFIX}")
    # Install the icon file in the proper location
    install (FILES images/icons/tuxmath.icns
      DESTINATION ${DESTDIR})
  else (APPLE)
    # This sets it relative to the install location (usually /usr/local/)
    set (DESTDIR "${TUXMATH_DATA_PREFIX}")
  endif (APPLE)
endif (WIN32)

message("Installing data to ${DESTDIR}")

install (DIRECTORY .
  DESTINATION ${DESTDIR}
  PATTERN Makefile* EXCLUDE
  PATTERN .svn EXCLUDE
  PATTERN CMakeLists.txt EXCLUDE
  PATTERN *~ EXCLUDE)
