## Makefile.am for tuxmath docs:
## Process with AutoMake:

EXTRA_DIST = 	changelog	\
	COPYING.txt	\
	INSTALL.txt	\
	OFL.txt	\
	README_DATA_LICENSES.TXT	\
	README.txt	\
	TODO.txt

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(prefix)/doc/$(PACKAGE)

	$(INSTALL_DATA) $(srcdir)/changelog $(DESTDIR)$(prefix)/doc/$(PACKAGE)
	$(INSTALL_DATA) $(srcdir)/COPYING.txt $(DESTDIR)$(prefix)/doc/$(PACKAGE)
	$(INSTALL_DATA) $(srcdir)/INSTALL.txt $(DESTDIR)$(prefix)/doc/$(PACKAGE)
	$(INSTALL_DATA) $(srcdir)/OFL.txt $(DESTDIR)$(prefix)/doc/$(PACKAGE)
	$(INSTALL_DATA) $(srcdir)/README_DATA_LICENSES.TXT $(DESTDIR)$(prefix)/doc/$(PACKAGE)
	$(INSTALL_DATA) $(srcdir)/README.txt $(DESTDIR)$(prefix)/doc/$(PACKAGE)
	$(INSTALL_DATA) $(srcdir)/TODO.txt $(DESTDIR)$(prefix)/doc/$(PACKAGE)

uninstall-local:
	-rm -f $(DESTDIR)$(prefix)/doc/$(PACKAGE)/changelog
	-rm -f $(DESTDIR)$(prefix)/doc/$(PACKAGE)/COPYING.txt
	-rm -f $(DESTDIR)$(prefix)/doc/$(PACKAGE)/INSTALL.txt
	-rm -f $(DESTDIR)$(prefix)/doc/$(PACKAGE)/OFL.txt
	-rm -f $(DESTDIR)$(prefix)/doc/$(PACKAGE)/README_DATA_LICENSES.TXT
	-rm -f $(DESTDIR)$(prefix)/doc/$(PACKAGE)/README.txt
	-rm -f $(DESTDIR)$(prefix)/doc/$(PACKAGE)/TODO.txt
	-rm -rf $(DESTDIR)$(prefix)/doc/$(PACKAGE)
