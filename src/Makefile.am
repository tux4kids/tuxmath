if BUILD_MINGW32
bin_PROGRAMS = TuxMath
else
TUXMATHRC =
bin_PROGRAMS = tuxmath
endif

TuxMath_SOURCES = tuxmath.c setup.c title.c game.c \
		options.c credits.c playsound.c \
                mathcards.c fileops.c tuxmathrc.c

tuxmath_SOURCES = tuxmath.c setup.c title.c game.c \
		options.c credits.c playsound.c \
                mathcards.c fileops.c


WINDRES=@WINDRES@

if BUILD_MINGW32
  DATA_PREFIX=@PACKAGE_DATA_DIR@
else
  DATA_PREFIX=${pkgdatadir}
endif

AM_CFLAGS=-Wall -g -DDATA_PREFIX=\"${DATA_PREFIX}\" -DDEBUG \
	-DVERSION=\"@NAME_VERSION@\" -D$(SOUND)SOUND

# How to make an RC file
tuxmathrc.o: tuxmathrc.rc
	$(WINDRES) -i $< -o $@

