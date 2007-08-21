## Makefile.am for tuxmath src/po:
## Process with Automake to create Makefile.in

datadir = @datadir@
localedir = $(datadir)/locale

EXTRA_DIST = nn.po \
	nb.po	\
	POTFILES.in	\
	update-po.sh

install-data-am:
    for i in *.po; do langcode=$$(echo $$i | sed 's/\.po//'); \
	  msgfmt -o $(localedir)/$$langcode/LC_MESSAGES/tuxmath.mo $$langcode.po; \
	done;
