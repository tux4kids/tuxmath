bin_PROGRAMS = tuxmath
tuxmath_SOURCES = tuxmath.c setup.c title.c game.c \
		options.c credits.c playsound.c \
                mathcards.c fileops.c
if BUILD_MINGW32
  DATA_PREFIX=@PACKAGE_DATA_DIR@
else
  DATA_PREFIX=${pkgdatadir}
endif

AM_CFLAGS=-Wall -g -DDATA_PREFIX=\"${DATA_PREFIX}\" -DDEBUG \
	-DVERSION=\"@NAME_VERSION@\" -D$(SOUND)SOUND

