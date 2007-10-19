MAKENSIS=@NSIS@

SUBDIRS = po	\
	intl	\
	src	\
	data	\
	docs

NSI_INSTALL_DIR=@NSI_INSTALL_DIR@
NSI_DLL_DIR=@NSI_DLL_DIR@
NSI_TUXMATH_CONF_DIR=@NSI_TUXMATH_CONF_DIR@
##datadir = @datadir@
##localedir = $(datadir)/locale


## Bundle in fonts for distribution tar.gz to be used without package manager:
## i.e. to make tarball to post for individual download - use 'make distcheck' for Debian.
## (thanks to Ralf Wildenhues <Ralf.Wildenhues@gmx.de> for automake help!)
dist_with_fonts:
	$(MAKE) $(AM_MAKEFLAGS) distdir=$(PACKAGE)_w_fonts-$(VERSION) dist_fonts='AndikaDesRevG.ttf' dist

# This rule is a brute-force kludge! 
install-data-local:
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/$(PACKAGE_DATA_DIR); then \
	  $(mkinstalldirs) $(DESTDIR)/$(pkgdatadir); \
	  cd $(srcdir)/$(PACKAGE_DATA_DIR) ; tar cf -  --exclude "Makefile.in" --exclude "*.in" --exclude "*~" --exclude \ "Makefile" --exclude "Makefile.am" --exclude CVS --exclude .xvpics --exclude "1[1-9].ogg"  --exclude "2?.ogg" --exclude "*.svn*" * \
           | ( cd $(DESTDIR)/$(pkgdatadir) ; tar xf -) ; cd .. ; \
          cd po ; ls;\
          for i in *.po; do langcode=$$(echo $$i | sed 's/\.po//'); \
	  mkdir -p $(DESTDIR)/$(localedir)/$$langcode/LC_MESSAGES ; msgfmt -o $(DESTDIR)/$(localedir)/$$langcode/LC_MESSAGES/tuxmath.mo $$langcode.po; \
	  done; \
	fi

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

# How's this for brute force?!
uninstall-local:
	-chmod -R a+w $(DESTDIR)$(prefix)/share/$(PACKAGE);
	-rm -rf $(DESTDIR)$(prefix)/share/$(PACKAGE);
	-for i in *.po; do langcode=$$(echo $$i | sed 's/\.po//'); \
	rm $(DESTDIR)/$(localedir)/$$langcode/LC_MESSAGES/tuxmath.mo; \
	done;

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
#	-rm tuxmath-@VERSION@-win32-installer.exe


ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = config.rpath m4/ChangeLog
