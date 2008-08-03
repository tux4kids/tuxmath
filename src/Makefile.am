## Makefile.am for tuxmath src:
## Process with Automake to create Makefile.in

# Support for gettext:
datadir = @datadir@
localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

AM_CFLAGS=-Wall -g -DDATA_PREFIX=\"${DATA_PREFIX}\" -DDEBUG \
	-DVERSION=\"@NAME_VERSION@\" -D$(SOUND)SOUND

AM_CPPFLAGS = -DLOCALEDIR=\"$(localedir)\" \
	-I../intl -I$(top_srcdir)/intl

LDADD = @LIBINTL@


if BUILD_MINGW32
bin_PROGRAMS = TuxMath
else
TUXMATHRC =
bin_PROGRAMS = tuxmath tuxmathadmin
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
	campaign.c	\
	fileops.c	\
	ConvertUTF.c	\
	SDL_extras.c	\
	lessons.c	\
	scandir.c	\
	pixels.c

TuxMath_SOURCES  = $(tuxmath_SOURCES) tuxmathrc.rc

tuxmathadmin_SOURCES = tuxmathadmin.c

EXTRA_DIST = 	credits.h 	\
		fileops.h 	\
		game.h		\
		highscore.h 	\
		mathcards.h 	\
		options.h	\
		setup.h		\
		titlescreen.h	\
		campaign.h	\
		tuxmath.h	\
		ConvertUTF.h	\
		SDL_extras.h	\
		lessons.h	\
		gettext.h	\
		scandir.h	\
		pixels.h	\
		compiler.h


WINDRES=@WINDRES@

if BUILD_MINGW32
  DATA_PREFIX=@PACKAGE_DATA_DIR@
else
  DATA_PREFIX=${pkgdatadir}
endif

# How to make an RC file
tuxmathrc.o: tuxmathrc.rc
	$(WINDRES) -i $< -o $@

noinst_HEADERS = highscore.h




