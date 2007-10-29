## Makefile.am for tuxmath - data/missions:
## Process with Automake to create Makefile.in

SUBDIRS = arcade	\
	lessons

EXTRA_DIST = demo\
	 options

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(prefix)/share/$(PACKAGE)/missions

	$(INSTALL_DATA) $(srcdir)/demo $(DESTDIR)$(prefix)/share/$(PACKAGE)/missions
	$(INSTALL_DATA) $(srcdir)/options $(DESTDIR)$(prefix)/share/$(PACKAGE)/missions

uninstall-local:
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/missions/demo
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/missions/options
	rm -rf $(DESTDIR)$(prefix)/share/$(PACKAGE)/missions
