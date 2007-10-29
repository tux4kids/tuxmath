## Makefile.am for tuxmath data/images:
## Process with AutoMake:

SUBDIRS = backgrounds	\
	cities		\
	comets 		\
	icons		\
	igloos		\
	penguins	\
	sprites		\
	status		\
	title		\
	tux

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(prefix)/share/$(PACKAGE)/images

uninstall-local:
	rm -rf $(DESTDIR)$(prefix)/share/$(PACKAGE)/images