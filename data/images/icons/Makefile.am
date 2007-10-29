## Makefile.am for tuxmath data/images/icons:
## Process with AutoMake:
EXTRA_DIST = tuxmath.ico	\
	icon.png	\
	icon.xpm	\
	tuxmath.icns

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(prefix)/share/$(PACKAGE)/images/icons

	$(INSTALL_DATA) $(srcdir)/tuxmath.ico $(DESTDIR)$(prefix)/share/$(PACKAGE)/images/icons
	$(INSTALL_DATA) $(srcdir)/icon.png $(DESTDIR)$(prefix)/share/$(PACKAGE)/images/icons
	$(INSTALL_DATA) $(srcdir)/icon.xpm $(DESTDIR)$(prefix)/share/$(PACKAGE)/images/icons
	$(INSTALL_DATA) $(srcdir)/tuxmath.icns $(DESTDIR)$(prefix)/share/$(PACKAGE)/images/icons

uninstall-local:
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/images/icons/tuxmath.ico
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/images/icons/icon.png
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/images/icons/icon.xpm
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/images/icons/tuxmath.icns
	rm -rf $(DESTDIR)$(prefix)/share/$(PACKAGE)/images/icons
