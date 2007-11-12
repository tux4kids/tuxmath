## Top-level Makefile.am for tuxmath:
## Process with Automake to create Makefile.in
SUBDIRS = docs	\
	intl	\
	po	\
	src	\
	data

ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = config.rpath m4/ChangeLog
datadir = @datadir@
localedir = $(datadir)/locale



MAKENSIS=@NSIS@
NSI_INSTALL_DIR=@NSI_INSTALL_DIR@
NSI_DLL_DIR=@NSI_DLL_DIR@
NSI_TUXMATH_CONF_DIR=@NSI_TUXMATH_CONF_DIR@


## Bundle in fonts for distribution tar.gz to be used without package manager:
## i.e. to make tarball to post for individual download - use 'make distcheck' for Debian.
## (thanks to Ralf Wildenhues <Ralf.Wildenhues@gmx.de> for automake help!)
dist_with_fonts:
	$(MAKE) $(AM_MAKEFLAGS) distdir=$(PACKAGE)_w_fonts-$(VERSION) dist_fonts='AndikaDesRevG.ttf' dist

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(prefix)/share/$(PACKAGE)

uninstall-local:
	rm -rf $(DESTDIR)$(prefix)/share/$(PACKAGE)


## FIXME make better use of autotools here:
install-nsi-local: all
	$(INSTALL) -d $(top_srcdir)/$(NSI_INSTALL_DIR)/$(PACKAGE_DATA_DIR);
	(cd $(top_srcdir)/$(PACKAGE_DATA_DIR) ; tar cf -  --exclude "Makefile.in" --exclude "*.in" --exclude "*~" --exclude \ "Makefile" --exclude "Makefile.am" --exclude CVS --exclude .xvpics --exclude "1[1-9].ogg"  --exclude "2?.ogg" --exclude "*.svn*" * ) \
         | ( cd $(top_srcdir)/$(NSI_INSTALL_DIR)/$(PACKAGE_DATA_DIR) ; tar xf -) ; \
	cp $(NSI_DLL_DIR)/*.dll $(top_srcdir)/$(NSI_INSTALL_DIR) ;
	cp $(top_srcdir)/src/TuxMath.exe $(top_srcdir)/$(NSI_INSTALL_DIR) ;

if NSI_USE_TUXMATH_CONF
	$(INSTALL) -d $(top_srcdir)/$(NSI_INSTALL_DIR)/TuxMathConfig ;
	cp $(NSI_TUXMATH_CONF_DIR)/*.*  $(top_srcdir)/$(NSI_INSTALL_DIR)/TuxMathConfig ;
endif 

install-nsi-am: install-nsi-local


nsis: install-nsi-local
if NSI_USE_TUXMATH_CONF
	$(MAKENSIS) -NOCD nsis/tuxmath_with_conf.nsi
else
	$(MAKENSIS) -NOCD nsis/tuxmath.nsi
endif

clean-local:
	@$(NORMAL_CLEAN)
	if test -d $(NSI_INSTALL_DIR); then \
	  rm -fr $(NSI_INSTALL_DIR); \
	fi
	-rm tuxmath-@VERSION@-win32-installer.exe


