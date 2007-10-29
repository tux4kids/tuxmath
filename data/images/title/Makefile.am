## Makefile.am for tuxmath data/images/title:
## Process with AutoMake:

EXTRA_DIST = 	menu_bkg.jpg	\
	title1.png

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(prefix)/share/$(PACKAGE)/images/title

	$(INSTALL_DATA) $(srcdir)/menu_bkg.jpg $(DESTDIR)$(prefix)/share/$(PACKAGE)/images/title
	$(INSTALL_DATA) $(srcdir)/title1.png $(DESTDIR)$(prefix)/share/$(PACKAGE)/images/title

uninstall-local:
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/images/title/menu_bkg.jpg
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/images/title/title1.png
	rm -rf $(DESTDIR)$(prefix)/share/$(PACKAGE)/images/title
