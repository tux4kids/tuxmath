MAKENSIS=@NSIS@

SUBDIRS = src

NSI_INSTALL_DIR=@NSI_INSTALL_DIR@
NSI_DLL_DIR=@NSI_DLL_DIR@
NSI_TUXMATH_CONF_DIR=@NSI_TUXMATH_CONF_DIR@

install-data-local:
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/$(PACKAGE_DATA_DIR); then \
	  $(mkinstalldirs) $(DESTDIR)/$(pkgdatadir); \
	  cd $(srcdir)/$(PACKAGE_DATA_DIR) ; tar cf -  --exclude "Makefile.in" --exclude "*.in" --exclude "*~" --exclude \ "Makefile" --exclude "Makefile.am" --exclude CVS --exclude .xvpics --exclude "1[1-9].ogg"  --exclude "2?.ogg" --exclude "*.svn*" * \
           | ( cd $(DESTDIR)/$(pkgdatadir) ; tar xf -) ; cd .. ; \
	fi

install-nsi-local: all
	$(INSTALL) -d $(top_srcdir)/$(NSI_INSTALL_DIR)/$(PACKAGE_DATA_DIR);
	(cd $(top_srcdir)/$(PACKAGE_DATA_DIR) ; tar cf -  --exclude "Makefile.in" --exclude "*.in" --exclude "*~" --exclude \ "Makefile" --exclude "Makefile.am" --exclude CVS --exclude .xvpics --exclude "1[1-9].ogg"  --exclude "2?.ogg" --exclude "*.svn*" * ) \
         | ( cd $(top_srcdir)/$(NSI_INSTALL_DIR)/$(PACKAGE_DATA_DIR) ; tar xf -) ; \
	cp $(NSI_DLL_DIR)/*.dll $(top_srcdir)/$(NSI_INSTALL_DIR) ; \
	cp $(SUBDIRS)/*.exe $(top_srcdir)/$(NSI_INSTALL_DIR) ;

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
#	-rm tuxmath-@VERSION@-win32-installer.exe

