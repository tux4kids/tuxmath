# Install the documentation

if (UNIX)
  if (APPLE)
    # The MACOSX_PACKAGE_LOCATION source file property doesn't seem to
    # work, so just do this by an explicit install command.
    set (DESTDIR "${TuxMath_BINARY_DIR}/src/${TUXMATH_DATA_PREFIX}/doc")
  else (APPLE)
    # This sets it relative to the install location (usually /usr/local/)
    set (DESTDIR "doc/tuxmath")
  endif (APPLE)
  install (DIRECTORY .
    DESTINATION ${DESTDIR}
    PATTERN Makefile* EXCLUDE
    PATTERN .svn EXCLUDE
    PATTERN CMakeLists.txt EXCLUDE
    PATTERN *~ EXCLUDE)
endif (UNIX)
