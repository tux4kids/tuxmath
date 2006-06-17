# Makefile for "tuxmath"
# (Hand-coded)

# Bill Kendrick
# bill@newbreedsoftware.com
# http://www.newbreedsoftware.com/tuxmath/

# Modified by David Bruce
# dbruce@tampabay.rr.com

# 2001.Aug.26 - 2006.Jun.12


CFLAGS=-Wall -g $(SDL_CFLAGS) -DDATA_PREFIX=\"$(DATA_PREFIX)\" -DDEBUG \
	-DVERSION=\"$(VERSION)\" -D$(SOUND)SOUND

SDL_CFLAGS=$(shell sdl-config --cflags)
SDL_LIBS=$(shell sdl-config --libs)

LIBS=$(SDL_LIBS) $(MIXERLIB) -lSDL_image
MIXERLIB=-lSDL_mixer

ifndef PREFIX
PREFIX=/usr/local
endif
DATA_PREFIX=$(DESTDIR)$(PREFIX)/share/tuxmath/
BIN_PREFIX=$(DESTDIR)$(PREFIX)/bin

# There isn't always a root group on every unix
OWNER=$(shell if `groups root | grep root > /dev/null` ; then echo root:root ; else echo root:wheel ; fi)


VERSION=tuxmath-0.8

all:	tuxmath

nosound:
	make tuxmath SOUND=NO MIXERLIB=

install:
	@echo "COPYING BINARY TO $(BIN_PREFIX)"
	mkdir -p $(BIN_PREFIX)
	mkdir -p $(DATA_PREFIX)
	strip tuxmath
	cp tuxmath $(BIN_PREFIX)
	chown $(OWNER) $(BIN_PREFIX)/tuxmath
	chmod 0755 $(BIN_PREFIX)/tuxmath
	@echo "COPYING DATA FILES TO $(DATA_PREFIX)"
	mkdir -p $(DATA_PREFIX)
	cp -r data/* $(DATA_PREFIX)
	chown -R $(OWNER) $(DATA_PREFIX)
	chmod a+Xr $(DATA_PREFIX)
	chmod a-w $(DATA_PREFIX)

uninstall:
	@echo "REMOVING TUX MATH"
	-rm $(BIN_PREFIX)/tuxmath
	-rm -r $(DATA_PREFIX)

clean:
	-rm tuxmath
	-rm obj/*.o
	-rmdir obj


tuxmath:	obj/tuxmath.o obj/setup.o obj/title.o obj/game.o \
		obj/options.o obj/credits.o obj/playsound.o \
                obj/mathcards.o
	@echo "LINKING!"
	$(CC) $(CFLAGS) $^ -o tuxmath $(LIBS)


obj:
	mkdir -p obj

obj/tuxmath.o:	src/tuxmath.c src/images.h src/sounds.h src/setup.h \
		src/title.h src/game.h src/options.h src/credits.h \
		src/playsound.h
	@echo "BUILDING tuxmath.o"
	-mkdir -p obj
	$(CC) $(CFLAGS) src/tuxmath.c -c -o obj/tuxmath.o

obj/setup.o:	src/setup.c src/setup.h src/sounds.h src/images.h src/game.h
	@echo "BUILDING setup.o"
	-mkdir -p obj
	$(CC) $(CFLAGS) src/setup.c -c -o obj/setup.o

obj/title.o:	src/title.c src/title.h src/setup.h src/sounds.h src/images.h \
		src/playsound.h
	@echo "BUILDING title.o"
	-mkdir -p obj
	$(CC) $(CFLAGS) src/title.c -c -o obj/title.o

obj/game.o:	src/game.c src/game.h src/setup.h src/sounds.h src/images.h \
		src/playsound.h
	@echo "BUILDING game.o"
	-mkdir -p obj
	$(CC) $(CFLAGS) src/game.c -c -o obj/game.o

obj/options.o:	src/options.c src/options.h src/images.h src/setup.h \
		src/sounds.h src/playsound.h
	@echo "BUILDING options.o"
	-mkdir -p obj
	$(CC) $(CFLAGS) src/options.c -c -o obj/options.o

obj/credits.o:	src/credits.c src/credits.h src/setup.h src/sounds.h \
		src/images.h
	@echo "BUILDING credits.o"
	-mkdir -p obj
	$(CC) $(CFLAGS) src/credits.c -c -o obj/credits.o

obj/playsound.o:	src/playsound.c src/playsound.h src/setup.h \
		src/sounds.h
	@echo "BUILDING playsound.o"
	-mkdir -p obj
	$(CC) $(CFLAGS) src/playsound.c -c -o obj/playsound.o

obj/mathcards.o:	src/mathcards.c src/mathcards.h 
	@echo "BUILDING mathcards.o"
	-mkdir -p obj
	$(CC) $(CFLAGS) src/mathcards.c -c -o obj/mathcards.o