## Makefile.am for tuxmath - data/fonts:

EXTRA_DIST = $(dist_fonts)

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(prefix)/share/$(PACKAGE)/fonts
	-$(INSTALL_DATA) $(srcdir)/AndikaDesRevG.ttf $(DESTDIR)$(prefix)/share/$(PACKAGE)/fonts

uninstall-local:
	-rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/fonts/AndikaDesRevG.ttf
	-rm -rf $(DESTDIR)$(prefix)/share/$(PACKAGE)/fonts
