#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_INIT([Tux Of Math Command],[2.0.4],[tuxmath-devel@lists.sourceforge.net],[tuxmath])

AC_PREREQ(2.61)
AC_CANONICAL_HOST
AC_CANONICAL_BUILD
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE(foreign)
AC_CONFIG_SRCDIR([src/tuxmath.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

NAME_VERSION=$PACKAGE-$VERSION

AC_DEFINE_UNQUOTED([PROGRAM_NAME], "$PACKAGE", [Tux Of Math Command])

AC_SUBST(NAME_VERSION)

AC_GNU_SOURCE

dnl For libtool support to link libt4k-common:
dnl NOTE - currently libtool init commented out because of errors in crossbuild.
dnl libtool not actually needed until we start to use libt4k-common.
dnl When crossbuild issues fixed, uncomment libtool init and remove ranlib line.
#LT_INIT
AC_PROG_RANLIB

# ----------------------------------------------------------------------
# Checks for programs.
# ----------------------------------------------------------------------

AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_MKDIR_P
AC_PROG_YACC


# ---------------------------------------------------------------------
# Gettext support:
# ---------------------------------------------------------------------

AM_GNU_GETTEXT_VERSION([0.18.1])
dnl Note that having a bundled intl/ directory is now deprecated
AM_GNU_GETTEXT([external])





# -----------------------------------------------------------------------
# Checks for libraries.
# -----------------------------------------------------------------------


dnl ---------------------------------------------------------------------------
dnl In the following checks we attempt to use pkg-config if available, falling
dnl back to standard autoconf macro as needed:
dnl ---------------------------------------------------------------------------


dnl Check for SDL2
PKG_CHECK_MODULES([SDL2],
  [sdl2 >= 2.0.0],
  [],
  [AC_MSG_ERROR([SDL2 version 2.0.0 or newer is required! http://www.libsdl.org])])

CFLAGS="$CFLAGS $SDL2_CFLAGS"
LIBS="$LIBS $SDL2_LIBS"




dnl Check for SDL_image: ------------------------------------------------------

PKG_CHECK_MODULES([SDL2_IMAGE],
  [SDL2_image >= 2.0.0],
  [],
  [AC_MSG_ERROR([SDL2_image not found! http://www.libsdl.org/projects/SDL_image])])

AC_DEFINE([HAVE_LIBSDL2_IMAGE],[1],[Define to 1 if you have the 'SDL2_image' library])
CFLAGS="$CFLAGS $SDL2_IMAGE_CFLAGS"
LIBS="$LIBS $SDL2_IMAGE_LIBS"




dnl Check for SDL_mixer: --------------------------------------------------------


PKG_CHECK_MODULES([SDL2_MIXER],
  [SDL2_mixer >= 2.0.0],
  [],
  [AC_MSG_ERROR([SDL2_mixer not found! http://www.libsdl.org/projects/SDL_mixer])])

AC_DEFINE([HAVE_LIBSDL2_MIXER],[1],[Define to 1 if you have the 'SDL2_mixer' library])
CFLAGS="$CFLAGS $SDL2_MIXER_CFLAGS"
LIBS="$LIBS $SDL2_MIXER_LIBS"


dnl Check for libRSVG and cairo. SVG support is enabled by default ----------------------

AC_ARG_WITH([rsvg],
	[AS_HELP_STRING([--without-rsvg],[do not use libRSVG even if available])],
	[with_rsvg=no],
	[with_rsvg=yes])

if test "x$with_rsvg" = xyes; then
PKG_CHECK_MODULES([RSVG],
	[librsvg-2.0 >= 2.18],
	[],
	[AC_MSG_FAILURE([libRSVG test failed (--without-rsvg to disable svg support)])])

CFLAGS="$CFLAGS $RSVG_CFLAGS"
LIBS="$LIBS $RSVG_LIBS"
fi

if test "x$with_rsvg" = xyes; then
PKG_CHECK_MODULES([CAIRO],
	[cairo >= 1.4.10],
	[],
	[AC_MSG_FAILURE([cairo test failed (--without-rsvg to disable svg support)])])

CFLAGS="$CFLAGS $CAIRO_CFLAGS"
LIBS="$LIBS $CAIRO_LIBS"
AC_DEFINE([HAVE_RSVG],[1],[Define to 1 if you have the `libRSVG` library])
fi



dnl Check for math functions - needed for SDL_extras: --------------------------------------------

AC_CHECK_LIB([m],
	[csin],
	[],
	[AC_MSG_ERROR([Math library not found - functions in <math.h> may not be available.])])




dnl Check for SDL2_net: --------------------------------------------------------

dnl SDL2_net is enabled by default.

AC_ARG_WITH([sdl2net],
  [AS_HELP_STRING([--without-sdl2net],
  [Do not use SDL2_net even if available])],
  [],
  [with_sdl2net=yes]
)

if test "x$with_sdl2net" != xno; then
  dnl We will need this for socket programming on Win32
  AC_CHECK_LIB([ws2_32],
               [WSAStartup],
               [],
               [AC_MSG_NOTICE([Windows socket library ws2_32 not found - (only needed on win32)])])

  PKG_CHECK_MODULES([SDL2_NET],
    [sdl2_net >= 2.0.0],
    [],
    [AC_MSG_NOTICE([SDL2_net not located by pkg-config, checking with AC_CHECK_LIB:])
     AC_CHECK_LIB([SDL2_net],
       [SDLNet_Init],
       [],
       [AC_MSG_FAILURE([SDL2_net test failed (--without-sdl2net to disable)])])])

  AC_DEFINE([HAVE_LIBSDL2_NET],[1],[Define to 1 if you have the 'SDL2_net' library])
  CFLAGS="$CFLAGS $SDL2_NET_CFLAGS"
  LIBS="$LIBS $SDL2_NET_LIBS"





dnl Check for libxml2: --------------------------------------------------------

PKG_CHECK_MODULES([XML2],
	[libxml-2.0],
	[],
	[AC_MSG_NOTICE([libxml2 not located by pkg-config, checking with AC CHECK LIB:])
	 AC_CHECK_LIB([xml2],
		[xmlParseFile],
		[],
		[AC_MSG_ERROR([libxml-2.0 not found: http://www.xmlsoft.org])])])

AC_DEFINE([HAVE_LIBXML2],[1],[Define to 1 if you have the `libxml-2.0` library])
CFLAGS="$CFLAGS $XML2_CFLAGS"
LIBS="$LIBS $XML2_LIBS"



dnl libt4kcommon is now required: --------------------------------------------

PKG_CHECK_MODULES([T4K_COMMON],
	[t4k_common],
	[],
	[AC_MSG_NOTICE([t4k_common not located by pkg-config, checking with AC CHECK LIB:])
	 AC_CHECK_LIB([t4k_common],
		[InitT4KCommon],
		[],
		[AC_MSG_ERROR([libt4k_common is required: git://git.debian.org/tux4kids/t4kcommon.git])])])

AC_DEFINE([HAVE_LIBT4K_COMMON],[1],[Define to 1 if you have the `t4k_common` library])
CFLAGS="$CFLAGS $T4K_COMMON_CFLAGS"
LIBS="$LIBS $T4K_COMMON_LIBS"



# --------------------------------------------------------------------------------------
# Checks for header files.
# --------------------------------------------------------------------------------------

AC_FUNC_ALLOCA
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([argz.h error.h errno.h fcntl.h float.h iconv.h inttypes.h langinfo.h libgen.h libintl.h limits.h locale.h malloc.h math.h pthread.h stddef.h stdint.h stdio_ext.h stdlib.h string.h strings.h sys/param.h unistd.h wchar.h])


# --------------------------------------------------------------------------------------------
# Checks for typedefs, structures, and compiler characteristics.
# --------------------------------------------------------------------------------------------

AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_C_RESTRICT
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T
AC_C_VOLATILE
AC_CHECK_TYPES([ptrdiff_t])



# -------------------------------------------------------------------------------------------
# Checks for library functions.
# -------------------------------------------------------------------------------------------


AC_FUNC_CLOSEDIR_VOID
AC_FUNC_ERROR_AT_LINE
#AC_FUNC_MALLOC
AC_FUNC_MMAP
#AC_FUNC_REALLOC
AC_FUNC_STAT
AC_FUNC_STRCOLL
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([__argz_count __argz_next __argz_stringify alphasort atexit basename bcopy fcntl floor fsync getcwd localeconv localtime_r memmove mempcpy memset Mix_Init mkdir munmap nl_langinfo scandir setlocale sqrt stpcpy strcasecmp strchr strcspn strdup strncasecmp strndup strrchr strstr strtoul])





dnl FIXME - get rid of remaining win32 conditional code

# Check platform - see if WinMain needed:
AC_MSG_CHECKING([for native Win32])
case "$host" in
  *-*-mingw*)
    native_win32=yes
    ;;
  *)
    native_win32=no
    ;;
esac
AC_MSG_RESULT([$native_win32])

AM_CONDITIONAL(BUILD_MINGW32, test $native_win32 = yes)

# Set the
MINGW32_PACKAGE_DATA_DIR="data"
AC_SUBST(MINGW32_PACKAGE_DATA_DIR)


if test $native_win32 = yes; then
   AC_DEFINE([BUILD_MINGW32], 1,[Native MinGW32 build])
   CFLAGS="$CFLAGS -D__GW32__"
   LIBS="$LIBS -luuid -lole32 -lwsock32 -mwindows"
   CPPFLAGS="$CPPFLAGS -idirafter $prefix/include/glibc"
   AC_PATH_PROG(WINDRES, "$target_alias-windres", no, [$PATH])
fi


# ------------------------------------------------------------
# Support for building NSIS Win32 installer:
# ------------------------------------------------------------

# This is the temporary DESTDIR target into which we do a "make install" before
# the files get copied into the NSIS installer:
NSI_INSTALL_DIR=mingw32
AC_SUBST(NSI_INSTALL_DIR)

# This is the path (relative to the TuxMath.exe binary) where the data files are
# placed within the TuxMath folder on the user's Windows machine:
MINGW32_PACKAGE_DATA_DIR="data"
AC_SUBST(MINGW32_PACKAGE_DATA_DIR)

AC_ARG_WITH([dll-directory],
            AS_HELP_STRING([--with-dll-directory=path], [provide location of DLL files needed for build  [$(NSI_DLL_DIR)]]),
            [dll_path=$withval],
            [dll_path=no])

AM_CONDITIONAL(NSI_BUILD, test $dll_path = no)

if test $dll_path != no; then
NSI_DLL_DIR=$dll_path
else
NSI_DLL_DIR=~/tuxmath_dll
fi
AC_SUBST(NSI_DLL_DIR)

AC_PATH_PROG(NSIS, makensis, [], [$PATH])
AC_PATH_PROGS(U2D, [todos unix2dos], [], [$PATH])

# ---------------------------------------------------------------
# Create Makefiles
# ---------------------------------------------------------------

AC_CONFIG_FILES([
Makefile
tuxmath.spec
tuxmath_preview.spec
data/Makefile
data/fonts/Makefile
data/images/Makefile
data/images/backgrounds/Makefile
data/images/cities/Makefile
data/images/comets/Makefile
data/images/icons/Makefile
data/images/igloos/Makefile
data/images/penguins/Makefile
data/images/sprites/Makefile
data/images/status/Makefile
data/images/title/Makefile
data/images/tux/Makefile
data/images/factoroids/Makefile
data/menus/Makefile
data/missions/Makefile
data/missions/arcade/Makefile
data/missions/campaign/Makefile
data/missions/campaign/ace/Makefile
data/missions/campaign/cadet/Makefile
data/missions/campaign/commando/Makefile
data/missions/campaign/ranger/Makefile
data/missions/campaign/scout/Makefile
data/missions/lessons/Makefile
data/missions/multiplay/Makefile
data/sounds/Makefile
doc/Makefile
nsis/tuxmath.nsi
src/Makefile
po/Makefile.in
])

AC_OUTPUT
