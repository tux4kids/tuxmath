## Top-level Makefile.am for tuxmath:
## Process with Automake to create Makefile.in
SUBDIRS = doc	\
	intl	\
	po	\
	src	\
	data

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = config.rpath m4/ChangeLog
datadir = @datadir@
localedir = $(datadir)/locale

## 'data_fonts' should contain all of the fonts in data/fonts. It needs to go
## in this Makefile.am before the 'dist_with_fonts' target for that target
## to work - if it is in data/fonts/Makefile.am, it does not get expanded
## before the 'dist' target starts:
data_fonts = AndikaDesRevG.ttf


## These are defined in configure.ac:
MAKENSIS=@NSIS@
NSIS_FOUND=@NSIS_FOUND@
NSI_INSTALL_DIR=@NSI_INSTALL_DIR@
NSI_DLL_DIR=@NSI_DLL_DIR@

NSI_TEMP_INSTALL_DIR=$(abs_top_builddir)/_instw32

## Bundle in fonts for distribution tar.gz to be used without package manager:
## i.e. to make tarball to post for individual download - use 'make distcheck' for Debian.
## (thanks to Ralf Wildenhues <Ralf.Wildenhues@gmx.de> for automake help!)
## 'dist_fonts' is in EXTRA_DATA but is empty by default.  With this target,
## 'dist_fonts' gets set to a list of the font files in data/fonts before making
## 'dist', causing the fonts to be included in the tar.gz.
dist_with_fonts:
	$(MAKE) $(AM_MAKEFLAGS) distdir=$(PACKAGE)_w_fonts-$(VERSION) \
  dist_fonts=$(data_fonts) dist


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
           | ( cd $(top_builddir)/$(NSI_INSTALL_DIR)/$(PACKAGE_DATA_DIR); \
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
## copy needed dll files into NSI_INSTALL_DIR:
	-cp $(NSI_DLL_DIR)/*.dll $(top_builddir)/$(NSI_INSTALL_DIR);


install-nsi-am: install-nsi-local


nsis: install-nsi-local
	if test ["x$(MAKENSIS)"="x"]; then \
	  echo "\nNSIS program not found!\n"; \
	else \
	  $(MAKENSIS) -NOCD nsis/tuxmath.nsi; \
	fi

clean-local:
	@$(NORMAL_CLEAN)
	if test -d $(NSI_INSTALL_DIR); then \
	  rm -fr $(NSI_INSTALL_DIR); \
	fi
	if test -e tuxmath-@VERSION@-win32-installer.exe; then \
	  rm tuxmath-@VERSION@-win32-installer.exe; \
	fi


