if BUILD_MINGW32
bin_PROGRAMS = TuxMath
else
TUXMATHRC =
bin_PROGRAMS = tuxmath
endif

TuxMath_SOURCES = tuxmath.c setup.c titlescreen.c game.c \
		options.c credits.c playsound.c highscore.c \
		theme.c loaders.c alphabet.c pause.c gettext.c audio.c \
                mathcards.c fileops.c tuxmathrc.rc

tuxmath_SOURCES = tuxmath.c setup.c titlescreen.c game.c \
		options.c credits.c playsound.c highscore.c \
		theme.c loaders.c alphabet.c pause.c gettext.c audio.c \
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


#------------------ Packaging rules for Mac OSX ------------------------
#---Many thanks to the developers (esp. Sidney Markowitz?) of Enigma,---
#---whose Makefiles were the template for this one.---------------------

bundle_name := $(top_builddir)/macbuild/TuxMath.app
# Special target to copy all needed game files to TuxMath.app/Contents/Resources.
# For Mac OS X bundle.
.PHONY: bundle
bundle:
	-rm -rf $(bundle_name)/Contents
	mkdir -p $(bundle_name)/Contents/MacOS
	mkdir -p $(bundle_name)/Contents/Resources/data/
	echo "APPL????" > $(bundle_name)/Contents/PkgInfo
	cp $(top_builddir)/Info.plist $(bundle_name)/Contents/
	cp $(top_builddir)/data/images/tuxmath.icns $(bundle_name)/Contents/Resources/
	tar -cf - --exclude .svn -C $(top_builddir)/data . | tar xpf - -C $(bundle_name)/Contents/Resources/data
	mkdir -p $(bundle_name)/Contents/lib
	mkdir -p $(bundle_name)/Contents/bin
	cp $(top_builddir)/tuxmath.sh $(bundle_name)/Contents/MacOS/tuxmath

# Special target to copy the binary to Enigma.app/Contents/MacOS.
.PHONY: bundle-fw
bundle-fw: tuxmath-fw-mosx
	cp tuxmath-fw $(bundle_name)/Contents/bin/tuxmath
	strip $(bundle_name)/Contents/bin/tuxmath

# Special target to copy the libraries from /sw/lib (from Fink)
.PHONY: bundle-libs
bundle-libs: tuxmath-fw-mosx
	$(top_builddir)/mac_cplibs.sh tuxmath-fw $(bundle_name)/Contents/lib/

# Special target to create a binary linked with Mac OS X frameworks
static_prefix := `$(SDL_CONFIG) --prefix`
.PHONY: tuxmath-fw-mosx
tuxmath-fw-mosx: $(tuxmath_OBJECTS) $(tuxmath_DEPENDENCIES)
	$(CXX) $(LDFLAGS) -o tuxmath-fw $(tuxmath_OBJECTS) $(tuxmath_DEPENDENCIES) -L$(static_prefix)/lib -lSDLmain -lSDL -lSDL_image -lSDL_mixer -framework Cocoa -framework OpenGL

# Special target to make a disk image of the bundled app
.PHONY: bundle-dmg
bundle-dmg:
	hdiutil create  -ov -srcfolder $(bundle_name) -volname TuxMath -imagekey zlib-level=6 $(top_builddir)/macbuild/TuxMath.dmg

noinst_HEADERS = highscore.h
