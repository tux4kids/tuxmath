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
  DATA_PREFIX=@MINGW32_PACKAGE_DATA_DIR@
else
  TUXMATHRC =
  bin_PROGRAMS = tuxmath tuxmathadmin generate_lesson
  DATA_PREFIX=${pkgdatadir}
endif


tuxmath_SOURCES = tuxmath.c \
	setup.c 	\
	titlescreen.c	\
	game.c 		\
	factoroids.c    \
	fileops_media.c \
	options.c	\
	credits.c	\
	highscore.c	\
	linewrap.c	\
	loaders.c	\
	audio.c 	\
	mathcards.c	\
	campaign.c	\
	multiplayer.c	\
	fileops.c	\
	convert_utf.c	\
	SDL_extras.c	\
	SDL_rotozoom.c	\
	lessons.c	\
	scandir.c	\
	pixels.c	\
	linebreak.c	\
	width.c	\
	u16-mbtouc-aux.c	\
	u16-mbtouc.c	\
	u16-mbtouc-unsafe-aux.c	\
	u16-mbtouc-unsafe.c	\
	u8-mbtouc-aux.c	\
	u8-mbtouc.c	\
	u8-mbtouc-unsafe-aux.c	\
	u8-mbtouc-unsafe.c	\
.	u8-uctomb-aux.c	\
	u8-uctomb.c


# HACK "TuxMath" is the Windows program, whereas "tuxmath" is the Unix program
TuxMath_SOURCES  = $(tuxmath_SOURCES) tuxmathrc.rc

tuxmathadmin_SOURCES = tuxmathadmin.c

generate_lesson_SOURCES = generate_lesson.c	\
		mathcards.c	\
		options.c	\
		fileops.c	\
		lessons.c

EXTRA_DIST = 	credits.h 	\
	factoroids.h	\
	fileops.h 	\
	game.h		\
	globals.h	\
	highscore.h 	\
	linewrap.h	\
	loaders.h	\
	mathcards.h 	\
	options.h	\
	setup.h		\
	titlescreen.h	\
	campaign.h	\
	multiplayer.h	\
	tuxmath.h	\
	convert_utf.h	\
	SDL_extras.h	\
	SDL_rotozoom.h	\
	lessons.h	\
	gettext.h	\
	scandir.h	\
	pixels.h	\
	compiler.h	 \
	lbrkprop.h	\
	localcharset.h	\
	streq.h	\
	unistr.h	\
	xsize.h	\
	linebreak.h	\
	unitypes.h	\
	uniwidth.h



WINDRES=@WINDRES@

# How to make an RC file
tuxmathrc.o: tuxmathrc.rc
	$(WINDRES) -i $< -o $@




