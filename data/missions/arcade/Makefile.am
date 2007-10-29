## Makefile.am for tuxmath - data/missions/arcade:
## Process with Automake to create Makefile.in

EXTRA_DIST = ace	\
	ranger	\
	scout	\
	space_cadet

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(prefix)/share/$(PACKAGE)/missions/arcade

	$(INSTALL_DATA) $(srcdir)/ace $(DESTDIR)$(prefix)/share/$(PACKAGE)/missions/arcade
	$(INSTALL_DATA) $(srcdir)/ranger $(DESTDIR)$(prefix)/share/$(PACKAGE)/missions/arcade
	$(INSTALL_DATA) $(srcdir)/scout $(DESTDIR)$(prefix)/share/$(PACKAGE)/missions/arcade
	$(INSTALL_DATA) $(srcdir)/space_cadet $(DESTDIR)$(prefix)/share/$(PACKAGE)/missions/arcade

uninstall-local:
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/images/missions/arcade/ace
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/images/missions/arcade/ranger
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/images/missions/arcade/scout
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/images/missions/arcade/space_cadet
	rm -rf $(DESTDIR)$(prefix)/share/$(PACKAGE)/missions/arcade

