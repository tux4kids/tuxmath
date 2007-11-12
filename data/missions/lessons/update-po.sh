#!/bin/sh

# Create a 'source' file holding all the descriptions for the missions.
cd ../../../../trunk/data/missions/lessons
rm -f ./descr_lessons
for i in ./lesson*; do
 head --lines=1 $i | sed 's/^# \(.*\)$/_("\1")/' >> descr_lessons;
done;

# Update the POT translation template file.
#intltool-update --pot --gettext-package=tuxmath

# Update all the PO translation files.
#for i in *.po ; do
#  echo $i
#  msgmerge $i tuxmath.pot > temp.tmp && mv -f temp.tmp $i
#done

