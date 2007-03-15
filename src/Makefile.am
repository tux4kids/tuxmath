if BUILD_MINGW32
bin_PROGRAMS = TuxMath
else
TUXMATHRC =
bin_PROGRAMS = tuxmath
endif

TuxMath_SOURCES = tuxmath.c setup.c titlescreen.c game.c \
		options.c credits.c playsound.c highscore.c \
		theme.c loaders.c alphabet.c pause.c audio.c \
                mathcards.c fileops.c tuxmathrc.rc

tuxmath_SOURCES = tuxmath.c setup.c titlescreen.c game.c \
		options.c credits.c playsound.c highscore.c \
		theme.c loaders.c alphabet.c pause.c audio.c \
                mathcards.c fileops.c

# SUBDIRS = po

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
