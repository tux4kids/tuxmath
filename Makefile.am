SUBDIRS = src

icondir=$(datadir)/pixmaps
icon_DATA = data/images/icon.png

install-data-local:
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/$(PACKAGE_DATA_DIR); then \
	  $(mkinstalldirs) $(DESTDIR)/$(pkgdatadir); \
	  cd $(srcdir)/$(PACKAGE_DATA_DIR) ; tar cf -  --exclude "Makefile.in" --exclude "*.in" --exclude "*~" --exclude "Makefile" --exclude "Makefile.am" --exclude CVS --exclude .xvpics --exclude "1[1-9].ogg"  --exclude "2?.ogg" * | ( cd $(DESTDIR)/$(pkgdatadir) ; tar xf -) ; cd .. ; \
	fi

