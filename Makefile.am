MAKENSIS=@NSIS@

SUBDIRS = src

NSI_INSTALL_DIR=@NSI_INSTALL_DIR@
NSI_DLL_DIR=@NSI_DLL_DIR@

MY_RECURSIVE_TARGETS = install-nsi

$(MY_RECURSIVE_TARGETS):
	@failcom='exit 1'; \
	for f in x $$MAKEFLAGS; do \
	  case $$f in \
	    *=* | --[!k]*);; \
	    *k*) failcom='fail=yes';; \
	  esac; \
	done; \
	dot_seen=no; \
	target=`echo $@ | sed s/-recursive//`; \
	list='$(SUBDIRS)'; for subdir in $$list; do \
	  echo "Making $$target in $$subdir"; \
	  if test "$$subdir" = "."; then \
	    dot_seen=yes; \
	    local_target="$$target-am"; \
	  else \
	    local_target="$$target"; \
	  fi; \
	  (cd $$subdir && $(MAKE) $(AM_MAKEFLAGS) $$local_target) \
	  || eval $$failcom; \
	done; \
	if test "$$dot_seen" = "no"; then \
	  $(MAKE) $(AM_MAKEFLAGS) "$$target-am" || exit 1; \
	fi; test -z "$$fail"


install-data-local:
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/$(PACKAGE_DATA_DIR); then \
	  $(mkinstalldirs) $(DESTDIR)/$(pkgdatadir); \
	  cd $(srcdir)/$(PACKAGE_DATA_DIR) ; tar cf -  --exclude "Makefile.in" --exclude "*.in" --exclude "*~" --exclude \ "Makefile" --exclude "Makefile.am" --exclude CVS --exclude .xvpics --exclude "1[1-9].ogg"  --exclude "2?.ogg" * \
          --exclude "*.svn*" | ( cd $(DESTDIR)/$(pkgdatadir) ; tar xf -) ; cd .. ; \
	fi

install-nsi-local: all
	$(INSTALL) -d $(top_srcdir)/$(NSI_INSTALL_DIR)/$(PACKAGE_DATA_DIR);
	(cd $(top_srcdir)/$(PACKAGE_DATA_DIR) ; tar cf -  --exclude "Makefile.in" --exclude "*.in" --exclude "*~" --exclude \ "Makefile" --exclude "Makefile.am" --exclude CVS --exclude .xvpics --exclude "1[1-9].ogg"  --exclude "2?.ogg" * ) \
        --exclude "*.svn*" | ( cd $(top_srcdir)/$(NSI_INSTALL_DIR)/$(PACKAGE_DATA_DIR) ; tar xf -) ; cd .. ; \
	cp $(NSI_DLL_DIR)/*.dll $(top_srcdir)/$(NSI_INSTALL_DIR)

install-nsi-am: install-nsi-local

nsis: install-nsi
	$(MAKENSIS) -NOCD nsis/tuxmath.nsi

clean-local:
	@$(NORMAL_CLEAN)
	if test -d $(NSI_INSTALL_DIR); then \
	  rm -fr $(NSI_INSTALL_DIR); \
	fi
	-rm tuxmath-@VERSION@-win32-installer.exe
