## Makefile.am for tuxmath - data/missions/lessons:
## Process with Automake to create Makefile.in

EXTRA_DIST = alarm.wav	\
	buzz.wav	\
	cheer.wav	\
	click.wav	\
	explosion.wav	\
	game2.mod	\
	game3.mod	\
	game.mod	\
	laser.wav	\
	pop.wav	\
	shieldsdown.wav	\
	sizzling.wav	\
	tock.wav	\
	towerclock.wav	\
	tuxi.ogg

install-data-local:
	$(MKDIR_P) $(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds

	$(INSTALL_DATA) $(srcdir)/alarm.wav	$(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds
	$(INSTALL_DATA) $(srcdir)/buzz.wav	$(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds
	$(INSTALL_DATA) $(srcdir)/cheer.wav	$(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds
	$(INSTALL_DATA) $(srcdir)/click.wav	$(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds
	$(INSTALL_DATA) $(srcdir)/explosion.wav	$(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds
	$(INSTALL_DATA) $(srcdir)/game2.mod	$(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds
	$(INSTALL_DATA) $(srcdir)/game3.mod	$(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds
	$(INSTALL_DATA) $(srcdir)/game.mod	$(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds
	$(INSTALL_DATA) $(srcdir)/laser.wav	$(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds
	$(INSTALL_DATA) $(srcdir)/pop.wav	$(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds
	$(INSTALL_DATA) $(srcdir)/shieldsdown.wav	$(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds
	$(INSTALL_DATA) $(srcdir)/sizzling.wav	$(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds
	$(INSTALL_DATA) $(srcdir)/tock.wav	$(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds
	$(INSTALL_DATA) $(srcdir)/towerclock.wav	$(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds
	$(INSTALL_DATA) $(srcdir)/tuxi.ogg $(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds

uninstall-local:
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds/alarm.wav
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds/buzz.wav	
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds/cheer.wav	
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds/click.wav	
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds/explosion.wav	
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds/game2.mod	
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds/game3.mod	
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds/game.mod	
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds/laser.wav	
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds/pop.wav	
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds/shieldsdown.wav	
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds/sizzling.wav	
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds/tock.wav	
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds/towerclock.wav
	rm -f $(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds/tuxi.ogg
	rm -rf $(DESTDIR)$(prefix)/share/$(PACKAGE)/sounds
