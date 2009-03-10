#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_INIT([Tux Of Math Command],[1.7.2],[tuxmath-devel@lists.sourceforge.net],[tuxmath])
AC_CONFIG_SRCDIR([src/tuxmath.c])

AC_PREREQ(2.61)
AC_CANONICAL_HOST
AC_CANONICAL_BUILD
AC_CANONICAL_TARGET

AM_INIT_AUTOMAKE(foreign)
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADER([config.h])

NAME_VERSION=$PACKAGE-$VERSION

AC_DEFINE_UNQUOTED([PROGRAM_NAME], "$PACKAGE", [Tux Of Math Command])

AC_SUBST(NAME_VERSION)

# Needed for gettext:
AC_GNU_SOURCE
AM_GNU_GETTEXT_VERSION([0.17])
#AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT
AM_GNU_GETTEXT_INTL_SUBDIR


# ----------------------------------------------------------------------
# Checks for programs.
# ----------------------------------------------------------------------

AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_AWK
AC_PROG_CPP
AC_PROG_MKDIR_P
AC_PROG_RANLIB
AC_PROG_YACC


# -----------------------------------------------------------------------
# Checks for libraries.
# -----------------------------------------------------------------------

# Check for SDL 
SDL_VERSION=1.2.0

AM_PATH_SDL($SDL_VERSION, :,
         AC_MSG_ERROR([SDL version $SDL_VERSION not found! http://www.libsdl.org]) 
) 

CFLAGS="$CFLAGS $SDL_CFLAGS" 


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

if test $native_win32 = yes; then
  LIBS="$LIBS -lmingw32 -lSDLmain -lSDL"
else
  LIBS="$LIBS $SDL_LIBS"
fi



dnl Check for SDL_image: 
AC_CHECK_LIB([SDL_image], 
             [IMG_Load],
             ,
             [AC_MSG_ERROR([SDL_image not found! http://www.libsdl.org/projects/SDL_image])])

dnl Check for SDL_mixer:. 
AC_CHECK_LIB([SDL_mixer], 
             [Mix_OpenAudio], 
             ,
             [AC_MSG_ERROR([SDL_mixer not found! http://www.libsdl.org/projects/SDL_mixer])])

dnl SDL_Pango is enabled by default.
dnl If SDL_Pango disabled at configure time, or if we can't find it,
dnl we look for SDL_ttf:

AC_ARG_WITH([sdlpango],
            [AS_HELP_STRING([--without-sdlpango],
            [don't use SDL_Pango even if available])],
            [with_sdlpango=no],
            [with_sdlpango=yes])

if test "x$with_sdlpango" != xno; then
AC_CHECK_LIB([SDL_Pango],
             [SDLPango_Init],
             ,
             [with_sdlpango=no;
              AC_MSG_FAILURE([SDL_Pango test failed (--without-sdlpango to disable)])])
fi

if [test "x$with_sdlpango" = xno]; then
AC_CHECK_LIB([SDL_ttf],
             [TTF_Init],
             ,
             [AC_MSG_ERROR([SDL_ttf not found! http://www.libsdl.org/projects/SDL_ttf])])
fi


dnl Check for (somewhat) higher-level math functions - needed for SDL_extras
AC_CHECK_LIB([m],
             [csin],
             ,
             [AC_MSG_ERROR([Math library not found - functions in <math.h> may not be available.])])




# --------------------------------------------------------------------------------------
# Checks for header files.
# --------------------------------------------------------------------------------------

AC_FUNC_ALLOCA
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([argz.h error.h errno.h fcntl.h float.h iconv.h inttypes.h langinfo.h libgen.h libintl.h limits.h locale.h malloc.h math.h stddef.h stdint.h stdio_ext.h stdlib.h string.h strings.h sys/param.h unistd.h wchar.h])


# --------------------------------------------------------------------------------------------
# Checks for typedefs, structures, and compiler characteristics.
# --------------------------------------------------------------------------------------------

AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
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
AC_CHECK_FUNCS([__argz_count __argz_next __argz_stringify atexit basename bcopy getcwd localtime_r memmove mempcpy memset mkdir munmap nl_langinfo scandir setlocale stpcpy strcasecmp strchr strcspn strdup strncasecmp strndup strstr strtoul])



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

NSI_INSTALL_DIR=mingw32
AC_SUBST(NSI_INSTALL_DIR)

NSI_DLL_DIR=~/tuxmath_dll
AC_ARG_WITH(dll-directory,AS_HELP_STRING([--with-dll-directory=path],[set the path where dll for TuxMath are [$(NSI_DLL_DIR)]]), dll_path=$withval, dll_path=no)

AM_CONDITIONAL(NSI_BUILD, test $dll_path = no)

if test $dll_path != no; then
NSI_DLL_DIR=$dll_path
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
