## Platform-specific installation material
if (APPLE)
  # Copy any needed libraries to the installation target
  macro (copyFramework headerDir destDir)
    string(REPLACE "/Headers" "" FRAMEWORK_SOURCE_DIR ${headerDir})
    install(DIRECTORY ${FRAMEWORK_SOURCE_DIR} DESTINATION ${destDir})
  endmacro(copyFramework)
  set(FRAMEWORK_DEST_DIR ${TuxMath_BINARY_DIR}/src/${TUXMATH_MACOSX_BUNDLE_NAME}.app/Contents/Frameworks)
  copyFramework(${SDL_INCLUDE_DIR} ${FRAMEWORK_DEST_DIR})
  copyFramework(${SDLMIXER_INCLUDE_DIR} ${FRAMEWORK_DEST_DIR})
  copyFramework(${SDLIMAGE_INCLUDE_DIR} ${FRAMEWORK_DEST_DIR})

  # Internationalization library
  if (TUXMATH_BUILD_INTL)
    install(FILES ${CMAKE_BINARY_DIR}/intl/libintl.a DESTINATION ${FRAMEWORK_DEST_DIR})
  endif (TUXMATH_BUILD_INTL)

  # Make the disk image
  install(CODE "file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/macdist)")
  install(CODE "execute_process(COMMAND mv ${CMAKE_BINARY_DIR}/src/tuxmath.app ${CMAKE_BINARY_DIR}/macdist/)")
  install(CODE "execute_process(COMMAND mv ${CMAKE_BINARY_DIR}/src/tuxmathadmin ${CMAKE_BINARY_DIR}/macdist/)")
  #install(CODE "execute_process(COMMAND hdiutil create  -ov -srcfolder ${CMAKE_BINARY_DIR}/src/tuxmath.app -volname TuxMath -imagekey zlib-level=6 ${CMAKE_BINARY_DIR}/TuxMath.dmg)")
  #install(CODE "execute_process(COMMAND hdiutil create  -ov -srcfolder ${CMAKE_BINARY_DIR}/src/tuxmathadmin.app -volname TuxMathAdmin -imagekey zlib-level=6 ${CMAKE_BINARY_DIR}/TuxMathAdmin.dmg)")
  install(CODE "execute_process(COMMAND hdiutil create  -ov -srcfolder ${CMAKE_BINARY_DIR}/macdist -volname TuxMath -imagekey zlib-level=6 ${CMAKE_BINARY_DIR}/TuxMath.dmg)")

endif (APPLE)

