## Makefile.am for tuxmath - data/missions/lessons:
## Process with Automake to create Makefile.in

lessonsdir = $(pkgdatadir)/missions/lessons

dist_lessons_DATA = lesson00 \
  lesson01 \
  lesson02 \
  lesson03 \
  lesson04 \
  lesson05 \
  lesson06 \
  lesson07 \
  lesson08 \
  lesson09 \
  lesson10 \
  lesson11 \
  lesson12 \
  lesson13 \
  lesson14 \
  lesson15 \
  lesson16 \
  lesson17 \
  lesson18 \
  lesson19 \
  lesson20 \
  lesson21 \
  lesson22 \
  lesson23 \
  lesson24 \
  lesson25 \
  lesson26 \
  lesson27 \
  lesson28 \
  lesson29 \
  lesson30 \
  lesson31 \
  lesson32 \
  lesson33 \
  lesson34 \
  lesson35 \
  lesson36 \
  lesson37 \
  lesson38 \
  lesson39 \
  lesson40 \
  lesson41 \
  lesson42 \
  lesson43 \
  lesson44 \
  lesson45 \
  lesson46 \
  lesson47 \
  lesson48 \
  lesson49 \
  lesson50 \
  lesson51 \
  lesson52 \
  lesson53 \
  lesson54 \
  lesson55 \
  lesson56 \
  lesson57

EXTRA_DIST = descr_lessons \
  update-po.sh

all: descr_lessons

# Create a 'source' file holding all the descriptions for the missions.
descr_lessons: $(dist_lessons_DATA)
	cd $(srcdir); ./update-po.sh
