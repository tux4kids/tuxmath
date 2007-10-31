## Makefile.am for tuxmath src:
## Process with Automake to create Makefile.in

# SUBDIRS = po (po now directly under trunk)

if BUILD_MINGW32
bin_PROGRAMS = TuxMath
else
TUXMATHRC =
bin_PROGRAMS = tuxmath
endif

tuxmath_SOURCES = tuxmath.c \
	setup.c 	\
	titlescreen.c	\
	game.c 		\
	options.c	\
	credits.c	\
	highscore.c	\
	loaders.c	\
	audio.c 	\
	mathcards.c	\
	fileops.c	\
	ConvertUTF.c	\
	SDL_extras.c	\
	lessons.c

TuxMath_SOURCES  = $(tuxmath_SOURCES) tuxmathrc.rc

EXTRA_DIST = 	credits.h 	\
		fileops.h 	\
		game.h		\
		highscore.h 	\
		mathcards.h 	\
		options.h	\
		setup.h		\
		titlescreen.h	\
		tuxmath.h	\
		ConvertUTF.h	\
		SDL_extras.h	\
		lessons.h	\
		gettext.h


WINDRES=@WINDRES@

if BUILD_MINGW32
  DATA_PREFIX=@PACKAGE_DATA_DIR@
else
  DATA_PREFIX=${pkgdatadir}
endif

datadir = @datadir@
localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

AM_CFLAGS=-Wall -g -DDATA_PREFIX=\"${DATA_PREFIX}\" -DDEBUG \
	-DVERSION=\"@NAME_VERSION@\" -D$(SOUND)SOUND

# How to make an RC file
tuxmathrc.o: tuxmathrc.rc
	$(WINDRES) -i $< -o $@

noinst_HEADERS = highscore.h
