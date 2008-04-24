# The information in the data directory has to be placed in a location
# that tuxmath can find

if (UNIX)
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
  install (DIRECTORY .
    DESTINATION ${DESTDIR}
    PATTERN Makefile* EXCLUDE
    PATTERN .svn EXCLUDE
    PATTERN CMakeLists.txt EXCLUDE
    PATTERN *~ EXCLUDE)
endif (UNIX)
