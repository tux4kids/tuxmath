## Makefile.am for tuxmath src:
## Process with Automake to create Makefile.in

# Support for gettext:
datadir = @datadir@
localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

AM_CFLAGS=-Wall -g -DDATA_PREFIX=\"${DATA_PREFIX}\" -DDEBUG \
	-D$(SOUND)SOUND

AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\"


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
    comets.c    \
    comets_graphics.c  \
	setup.c 	\
	titlescreen.c	\
	menu.c		\
	menu_lan.c	\
	game.c 		\
	factoroids.c    \
	factoroids_graphics.c    \
	fileops_media.c \
	frame_counter.c \
	options.c	\
	credits.c	\
	draw_utils.c	\
	highscore.c	\
	audio.c 	\
        network.c       \
	mathcards.c	\
	campaign.c	\
	multiplayer.c	\
	fileops.c	\
	SDL_rotozoom.c	\
	lessons.c	\
	server.c


# HACK "TuxMath" is the Windows program, whereas "tuxmath" is the Unix program
TuxMath_SOURCES  = $(tuxmath_SOURCES) tuxmathrc.rc


tuxmathadmin_SOURCES = tuxmathadmin.c

generate_lesson_SOURCES = generate_lesson.c	\
		mathcards.c	\
		options.c	\
		fileops.c	\
		lessons.c

tuxmathserver_SOURCES = servermain.c	\
		server.c \
		mathcards.c	\
		options.c

tuxmathtestclient_SOURCES = testclient.c \
                            network.c  \
                            options.c  \
                            mathcards.c

EXTRA_DIST = 	\
    comets.h    \
    comets_graphics.h  \
    credits.h 	\
	draw_utils.h	\
	factoroids.h	\
	factoroids_graphics.h    \
	fileops.h 	\
	frame_counter.h	\
	game.h		\
	menu.h		\
	menu_lan.h	\
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
	compiler.h	\
	server.h	\
	testclient.h	\
	transtruct.h	\
        CMakeLists.txt
        


WINDRES=@WINDRES@

# How to make an RC file
tuxmathrc.o: tuxmathrc.rc
	$(WINDRES) -i $< -o $@
