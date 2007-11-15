#!/bin/sh

# Create a 'source' file holding all the descriptions for the missions.
# NOTE this script needs to be kept in the same dir as the lesson files
rm -f ./descr_lessons
for i in ./lesson*; do
 head --lines=1 $i | sed 's/^# \(.*\)$/_("\1")/' >> descr_lessons;
done;



