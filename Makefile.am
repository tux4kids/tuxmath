## Top-level Makefile.am for tuxmath:
## Process with Automake to create Makefile.in


## ---------------------------------------------------------------------------
## Variables: ----------------------------------------------------------------
## ---------------------------------------------------------------------------


SUBDIRS = data	\
	doc	\
	po	\
	src

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = t4k_common.mk \
	tuxmath.spec \
	tuxmath.desktop \
	config.rpath \
	m4/ChangeLog \
	Info.plist \
	CMakeLists.txt \
        cmake-modules/ConfigureChecks.cmake \
        cmake-modules/ConfigureChecksIntl.cmake \
        cmake-modules/FindIconv.cmake \
        cmake-modules/FindMSGFMT.cmake \
        cmake-modules/FindSDL_gfx.cmake \
        cmake-modules/FindSDL_Pango.cmake \
        cmake-modules/FindTux4Kids-common.cmake \
        cmake-modules/T4K_Sugar.cmake \
        po/CMakeLists.txt \
	config.h.cmake \
	buildw32/cross-configure.sh \
	buildw32/cross-make.sh \
	buildw32/setup_mingw-cross-env.sh \
	buildw32/tmwin.sh \
	buildw32/t4k_common.mk

##datadir = @datadir@
##localedir = $(datadir)/locale


## These are defined in configure.ac:
##MAKENSIS=$(NSIS)
##NSIS_FOUND=@NSIS_FOUND@
##NSI_INSTALL_DIR=@NSI_INSTALL_DIR@
##NSI_DLL_DIR=@NSI_DLL_DIR@
NSI_TEMP_INSTALL_DIR=$(abs_top_builddir)/_instw32

## ---------------------------------------------------------------------------
## Rules: --------------------------------------------------------------------
## ---------------------------------------------------------------------------


## Bundle in fonts for distribution tar.gz to be used without package manager:
## i.e. to make tarball to post for individual download. 
## 
## for tarball with fonts: use 'make dist_with_fonts' (for *tar.gz)
## or 'make dist_with_fonts_bzip2 (for *tar.bzip2)
## for tarball without fonts: (e.g. for Debian packaging) - use 'make distcheck'
##
## (thanks to Ralf Wildenhues <Ralf.Wildenhues@gmx.de> for automake help!)
## 'dist_fonts' is in EXTRA_DATA but is empty by default.  With this target,
## 'dist_fonts' gets set to a list of the font files in data/fonts before making
## 'dist', causing the fonts to be included in the tar.gz.
## 'data_fonts' should contain all of the fonts in data/fonts. It needs to go
## in this Makefile.am before the 'dist_with_fonts' target for that target
## to work - if it is in data/fonts/Makefile.am, it does not get expanded
## before the 'dist' target starts:
data_fonts='AndikaDesRevG.ttf'
dist_with_fonts:
	$(MAKE) $(AM_MAKEFLAGS) distdir=$(PACKAGE)_w_fonts-$(VERSION) \
           dist_fonts=$(data_fonts) dist
dist_with_fonts_bzip2:
	$(MAKE) $(AM_MAKEFLAGS) distdir=$(PACKAGE)_w_fonts-$(VERSION) \
           dist_fonts=$(data_fonts) dist-bzip2




## For building the NSIS executable Win32 installer - this rule first
## does a "make install" into NSI_TEMP_INSTALL_DIR, which results in
## a local copy of the complete unix-style install.
## Subsequent commands then copy the needed files into NSI_INSTALL_DIR,
## which has the exact directory structure of the self-contained
## 'TuxMath' folder that gets installed onto the Windows machine.

install-nsi-local: all
## create NSI_TEMP_INSTALL_DIR and install to that location:
	$(INSTALL) -d $(NSI_TEMP_INSTALL_DIR)
	$(MAKE) $(AM_MAKEFLAGS) DESTDIR=$(NSI_TEMP_INSTALL_DIR) install
## create NSI_INSTALL_DIR/data dir and copy data files to that location:
	$(INSTALL) -d $(top_builddir)/$(NSI_INSTALL_DIR)/data;
	(cd $(NSI_TEMP_INSTALL_DIR)/$(pkgdatadir); \
           tar cf -  * ) \
           | ( cd $(top_builddir)/$(NSI_INSTALL_DIR)/data; \
           tar xf -)
## create NSI_INSTALL_DIR/doc dir and copy docs to that location:
	$(INSTALL) -d $(top_builddir)/$(NSI_INSTALL_DIR)/doc;
	(cd $(NSI_TEMP_INSTALL_DIR)/$(docdir); \
           tar cf -  * ) \
           | ( cd $(top_builddir)/$(NSI_INSTALL_DIR)/doc; \
           tar xf -)
## create NSI_INSTALL_DIR/locale dir and copy locales to that location:
	$(INSTALL) -d $(top_builddir)/$(NSI_INSTALL_DIR)/locale;
	(cd $(NSI_TEMP_INSTALL_DIR)/$(localedir); \
           tar cf -  * ) \
           | ( cd $(top_builddir)/$(NSI_INSTALL_DIR)/locale; \
           tar xf -)
## copy executable into NSI_INSTALL_DIR:
	(cd $(NSI_TEMP_INSTALL_DIR)/$(bindir); \
           mv *TuxMath.exe TuxMath.exe; \
           tar cf - TuxMath.exe  ) \
           | ( cd $(top_builddir)/$(NSI_INSTALL_DIR); \
           tar xf -)
## Done with NSI_TEMP_INSTALL_DIR so uninstall:
	$(MAKE) $(AM_MAKEFLAGS) DESTDIR=$(NSI_TEMP_INSTALL_DIR) uninstall
	rm -rf $(NSI_TEMP_INSTALL_DIR)
## Convert text files from Unix format DOS/Windows format and rename
## them with ".txt" extension:
	(cd $(top_builddir)/$(NSI_INSTALL_DIR)/doc; \
	FILES=`find . -type f`; \
	for file in $$FILES; do \
	    echo "Processing "$$file; \
	    $(U2D) $$file; \
	    mv $$file $$file".txt"; \
	done)

	(cd $(top_builddir)/$(NSI_INSTALL_DIR)/data/missions; \
	FILES=`find . -type f`; \
	for file in $$FILES; do \
	    echo "Processing "$$file; \
	    $(U2D) $$file; \
	    mv $$file $$file".txt"; \
	done)

## NOTE: we don't use any dll files now that we use mingw-cross-env
## because the entire executable is statically linked.
## copy needed dll files into NSI_INSTALL_DIR:
#	-cp $(NSI_DLL_DIR)/*.dll $(top_builddir)/$(NSI_INSTALL_DIR);

## NOTE: However, we do need to copy non-executable data and config files
## needed by the libraries that we have used:

## Copy data files for our t4k_common library:
	(cd $(prefix)/share/t4k_common; \
	   tar cf - * ) \
           | ( cd $(top_builddir)/$(NSI_INSTALL_DIR)/data; \
           tar xf -)
	echo 'fontconfig files: ' $(sysconfdir)/fonts	

## Create NSI_INSTALL_DIR/fonts dir and copy fontconfig's needed
## fonts dir to that location. 

## NOTE: $(sysconfdir) will point to the etc dir 
## in the mingw-cross-env installation, where the fontconfig lib will have
## installed a dir called "fonts" containing the libs config files, most
## importantly fonts.conf.  This folder must be copied into the same dir on
## the user's Windows machine as the final TuxMath.exe binary.  It would be 
## desirable to be able to set this to a location of our choice - DSB.
	$(INSTALL) -d $(top_builddir)/$(NSI_INSTALL_DIR)/fonts;
	(cd $(sysconfdir)/fonts; \
           tar cf -  * ) \
           | ( cd $(top_builddir)/$(NSI_INSTALL_DIR)/fonts; \
           tar xf -)


install-nsi-am: install-nsi-local


nsis:

	if test "x$(U2D)" = "x"; then \
	  echo "\nWarning - unix2dos (or todos) program not found!"; \
          echo "It is needed to convert text files to Windows format."; \
          echo "Without this program, user documention will be in Unix" ; \
          echo "format, which will not display correctly with common"; \
          echo "Windows text editors such as Notepad\n"; \
	fi

	if test "x$(NSIS)" = "x"; then \
	  echo "\nError - makensis program not found!"; \
          echo "It is needed to build the NSIS executable installer for Windows."; \
          echo "Install it from your distribution's repository, or" ; \
          echo "download it from: http://nsis.sourceforge.net/Download\n"; \
	else \
	$(MAKE) $(AM_MAKEFLAGS) install-nsi-local; \
	  $(NSIS) -NOCD nsis/tuxmath.nsi; \
	fi


clean-local:
	@$(NORMAL_CLEAN)
	if test -d $(NSI_INSTALL_DIR); then \
	  rm -fr $(NSI_INSTALL_DIR); \
	fi
	if test -e tuxmath-@VERSION@-win32-installer.exe; then \
	  rm tuxmath-@VERSION@-win32-installer.exe; \
	fi

.PHONY: nsis

