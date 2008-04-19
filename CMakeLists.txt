project (tuxmath)

message("CMAKE_INSTALL_PREFIX is ${CMAKE_INSTALL_PREFIX}")

set (TUXMATH_VERSION "1.6.1")
set (TUXMATHADMIN_VERSION "0.1.1")

## Define the extra paths
set (DATA_TUXMATH ${tuxmath_SOURCE_DIR}/data)
set (DOC_TUXMATH ${tuxmath_SOURCE_DIR}/doc)
set (PO_TUXMATH ${tuxmath_SOURCE_DIR}/po)

## Set up OS-specific path information
if (APPLE)
  set(DATA_PREFIX TuxMath.app/Contents/Resources/data)
  set(LOCALEDIR TuxMath.app/Contents/Resources/locale)
  set_source_files_properties (
    ${DATA_TUXMATH}
    PROPERTIES
    MACOSX_PACKAGE_LOCATION Resources
    )
elseif (UNIX)
  # A non-apple Unix (Linux, Solaris, etc.)
  set (DATA_PREFIX ${CMAKE_INSTALL_PREFIX}/share/tuxmath)
  set (LOCALEDIR ${CMAKE_INSTALL_PREFIX}/share/locale)
endif (APPLE)

## Code for internationalization: if we don't have gettext, we need to
## compile the intl library
include (CheckSymbolExists)
check_symbol_exists (gettext libintl.h HAVE_GETTEXT)
set(CMAKE_MODULE_PATH "${tuxmath_SOURCE_DIR}")
find_package (MSGFMT)

## Add subdirectories
add_subdirectory (src)
if (HAVE_GETTEXT)
  add_subdirectory (po)
endif (HAVE_GETTEXT)


