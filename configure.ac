#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_INIT([Tux Of Math Command],[1.8.0],[tuxmath-devel@lists.sourceforge.net],[tuxmath])

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
LT_INIT

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

AM_GNU_GETTEXT_VERSION([0.17])

dnl Use the following line whenever possible:
AM_GNU_GETTEXT([external])
dnl If external intl lib unavailable on platform, comment out the
dnl above line and uncomment the following line to use the included
dnl intl subdirectory. This used to be needed for the Windows cross-
dnl build but is not longer necessary:
#AM_GNU_GETTEXT

# -----------------------------------------------------------------------
# Checks for libraries.
# -----------------------------------------------------------------------

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


dnl ---------------------------------------------------------
dnl For crossbuild with mingw-cross-env, we use pkg_config,
dnl but for native linux build we use #AC_CHECK_LIB() because
dnl some distros (incl. Debian) don't support pkg_config for
dnl the SDL libraries:
dnl ---------------------------------------------------------

AS_IF([test "x$native_win32" = xyes],
dnl What we do for mingw crossbuild:
[
	# Check for SDL, SDL_image, SDL_mixer and SDL_Pango
	[echo "Mingw crossbuild detected - using pkgconfig to check for SDL libs:"],
	PKG_CHECK_MODULES([SDL], [sdl >= 1.2.0 SDL_image SDL_mixer SDL_Pango SDL_net])
	CFLAGS="$CFLAGS $SDL_CFLAGS" 
	LIBS="$LIBS $SDL_LIBS"
],
[test "x$native_win32" = xno],
dnl What we do if not doing mingw crossbuild:
[
	# Check for SDL:
        [echo "Mingw crossbuild not detected - using #AC_CHECK_LIB() for SDL libs:"],
	SDL_VERSION=1.2.0
	AM_PATH_SDL($SDL_VERSION, :,
		 AC_MSG_ERROR([SDL version $SDL_VERSION not found! http://www.libsdl.org]) 
	) 
	CFLAGS="$CFLAGS $SDL_CFLAGS" 
	LIBS="$LIBS $SDL_LIBS"

	# Check for SDL_image: 
	AC_CHECK_LIB([SDL_image], 
		     [IMG_Load],
		     ,
		     [AC_MSG_ERROR([SDL_image not found! http://www.libsdl.org/projects/SDL_image])])

	# Check for SDL_mixer:. 
	AC_CHECK_LIB([SDL_mixer], 
		     [Mix_OpenAudio], 
		     ,
		     [AC_MSG_ERROR([SDL_mixer not found! http://www.libsdl.org/projects/SDL_mixer])])


	dnl SDL_Pango is enabled by default.
	dnl If SDL_Pango disabled at configure time, or if we can't find it,
	dnl we look for SDL_ttf:

	AC_ARG_WITH([sdlpango],
		    [AS_HELP_STRING([--without-sdlpango],
		    [Do not use SDL_Pango even if available])],
		    [],
		    [with_sdlpango=yes])

	if test "x$with_sdlpango" != xno; then
	AC_CHECK_LIB([SDL_Pango],
		     [SDLPango_Init],
		     ,
		     [with_sdlpango=no;
		      AC_MSG_FAILURE([SDL_Pango test failed (--without-sdlpango to disable)])])
	fi
],
dnl This message means the test variable was not set to either "yes" or "no"
[echo Warning - mingw crossbuild status not specified])

dnl ----------------------------------------------------------------------------
dnl End of AS_IF() conditional -------------------------------------------------
dnl ---------------------------------------------------------------------------



AC_CHECK_LIB([SDL_ttf],
             [TTF_Init],
             ,
             [AC_MSG_ERROR([SDL_ttf not found! http://www.libsdl.org/projects/SDL_ttf])])

dnl Check for libRSVG and cairo. SVG support is enabled by default

AC_ARG_WITH([rsvg],
            [AS_HELP_STRING([--without-rsvg],[Do not use libRSVG even if available])],
            [with_rsvg=no],
            [with_rsvg=yes])

if test "x$with_rsvg" = xyes; then
  RSVG_CFLAGS=""
  RSVG_LIBS=""
  PKG_CHECK_MODULES([RSVG],
                    [librsvg-2.0 >= 2.18],
                    ,
                    [AC_MSG_FAILURE([libRSVG test failed (--without-rsvg to disable svg support)])])
  CFLAGS="$CFLAGS $RSVG_CFLAGS"
  LIBS="$LIBS $RSVG_LIBS"
fi

if test "x$with_rsvg" = xyes; then
  CAIRO_CFLAGS=""
  CAIRO_LIBS=""
  PKG_CHECK_MODULES([CAIRO],
                    [cairo >= 1.4.10],
                    ,
                    [AC_MSG_FAILURE([cairo test failed (--without-rsvg to disable svg support)])])
  CFLAGS="$CFLAGS $CAIRO_CFLAGS"
  LIBS="$LIBS $CAIRO_LIBS"
  AC_DEFINE([HAVE_RSVG],[1],[Define to 1 if you have the `libRSVG` library])
fi

dnl Check for (somewhat) higher-level math functions - needed for SDL_extras
AC_CHECK_LIB([m],
              [csin],
              ,
              [AC_MSG_ERROR([Math library not found - functions in <math.h> may not be available.])])

dnl Use of libt4kcommon is disabled for now by default 
dnl but can be enabled at configure time for testing and development.

AC_ARG_WITH([libt4kcommon],
            [AS_HELP_STRING([--with-libt4kcommon],[use libtk4common if available])],
            [with_libt4kcommon=yes],
            [with_libt4kcommon=no])

if test "x$with_libt4kcommon" != xno; then
AC_CHECK_LIB([t4kcommon],
             [InitT4KCommon],
             ,
             [with_libt4kcommon=no;
              AC_MSG_FAILURE([libt4kcommon test failed (--without-libt4kcommon to disable use of libt4kcommon)])])
fi


dnl Check for (somewhat) higher-level math functions - needed for SDL_extras
dnl AC_CHECK_LIB([m],
dnl              [csin],
dnl              ,
dnl              [AC_MSG_ERROR([Math library not found - functions in <math.h> may not be available.])])



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
AC_CHECK_FUNCS([__argz_count __argz_next __argz_stringify atexit basename bcopy fcntl floor getcwd localeconv localtime_r memmove mempcpy memset mkdir munmap nl_langinfo scandir setlocale sqrt stpcpy strcasecmp strchr strcspn strdup strncasecmp strndup strrchr strstr strtoul])


# Set the 
MINGW32_PACKAGE_DATA_DIR="data"
AC_SUBST(MINGW32_PACKAGE_DATA_DIR)

#AM_CONDITIONAL(BUILD_MINGW32, test "$native_win32" = yes)

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
nsis/tuxmath_with_conf.nsi
src/Makefile
intl/Makefile
linebreak/Makefile
po/Makefile.in
])

AC_OUTPUT
