## Makefile.am for tuxmath src:
## Process with Automake to create Makefile.in

# Support for gettext:
datadir = @datadir@
localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

AM_CFLAGS=-Wall -g -DDATA_PREFIX=\"${DATA_PREFIX}\" -DDEBUG \
	-D$(SOUND)SOUND

AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\"

#NOTE we can remove the following starting with t4k_common 0.0.4
LDADD = @LIBINTL@ 

if BUILD_MINGW32
  bin_PROGRAMS = TuxMath tuxmathserver
  DATA_PREFIX=@MINGW32_PACKAGE_DATA_DIR@
else
  TUXMATHRC =
  bin_PROGRAMS = tuxmath \
                 tuxmathadmin \
                 generate_lesson \
                 tuxmathserver	\
                 tuxmathtestclient

  DATA_PREFIX=${pkgdatadir}
endif


tuxmath_SOURCES = tuxmath.c \
	setup.c 	\
	titlescreen.c	\
	menu.c		\
	game.c 		\
	factoroids.c    \
	fileops_media.c \
	options.c	\
	credits.c	\
	highscore.c	\
	audio.c 	\
        network.c       \
	mathcards.c	\
	campaign.c	\
	multiplayer.c	\
	fileops.c	\
	SDL_rotozoom.c	\
	lessons.c	\
	pixels.c	\
	server.c	\
	throttle.c	


# HACK "TuxMath" is the Windows program, whereas "tuxmath" is the Unix program
TuxMath_SOURCES  = $(tuxmath_SOURCES) tuxmathrc.rc


# NOTE - the "extra" programs built here don't need the same linking,
# so they have their own prog_LDADD statements to override the
# global LDADD flag.

tuxmathadmin_SOURCES = tuxmathadmin.c
tuxmathadmin_LDADD = @LIBINTL@

generate_lesson_SOURCES = generate_lesson.c	\
		mathcards.c	\
		options.c	\
		fileops.c	\
		lessons.c
generate_lesson_LDADD =


tuxmathserver_SOURCES = servermain.c	\
		server.c \
		mathcards.c	\
		throttle.c	\
		options.c
tuxmathserver_LDADD = @LIBINTL@

tuxmathtestclient_SOURCES = testclient.c \
                            throttle.c \
                            network.c  \
                            options.c  \
                            mathcards.c
tuxmathtestclient_LDADD =

EXTRA_DIST = 	credits.h 	\
	factoroids.h	\
	fileops.h 	\
	game.h		\
	menu.h		\
	globals.h	\
	highscore.h 	\
        network.h       \
	titlescreen.h   \
	menu.h		\
	options.h	\
	setup.h		\
	mathcards.h 	\
	campaign.h	\
	multiplayer.h	\
	tuxmath.h	\
	SDL_rotozoom.h	\
	lessons.h	\
	gettext.h	\
	pixels.h	\
	compiler.h	\
	server.h	\
	testclient.h	\
	transtruct.h	\
	throttle.h      \
        CMakeLists.txt
        


WINDRES=@WINDRES@

# How to make an RC file
tuxmathrc.o: tuxmathrc.rc
	$(WINDRES) -i $< -o $@
