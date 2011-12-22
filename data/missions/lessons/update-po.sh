#!/bin/sh

# Create a 'source' file holding all the descriptions for the missions.
# That file (descr_lessons) is listed in po/POTFILES.in so the strings will go into
# tuxmath.pot and subsequently into the individual po files for translation.
# NOTE this script needs to be kept in the same dir as the lesson files
rm -f ./descr_lessons
for i in ./lesson*; do
 head --lines=1 $i | sed 's/^# \(.*\)$/_("\1")/' >> descr_lessons;
done;



