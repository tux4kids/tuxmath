.. image:: img/tux_helmet.png
   :align: right

More Information
================

Shortcut Keys
-------------

There are a number of shortcut keys that can be used to do various things
in-game:

* [F10]
	Switches between windowed and full-screen display mode.
* [P] or [TAB]
	Pauses the game on gamemodes where pausing is allowed: the Training
	Academy modes allow pausing while the Arcade games do not.
* [UP ARROW] or [DOWN ARROW]
	Speeds up or slows down the game by 20%. This function is allowed on
	gamemodes where pausing is also allowed.
* [ESC]
	Quits the current game and displays the menu.

.. _configuration:

Custom Configuration
--------------------

TuxMath stores an options file in your home area which allows you
exstensive control over the program, particularly with respect to the way
questions are generated.

On Windows XP, it is located at "C:\\Documents and Settings\\USER\\Application
Data\\TuxMath\\options.txt". If you cannot find Application Data, remember that
it is hidden by the operating system by default.

On Unix\\Linux, it is located at "/home/USER/.tuxmath/options".

On Macs, it can be found under "tuxmath/Contents/Resources".

This file is heavily documented throughout, and I will not go over it all here.

Tracking Performance
--------------------

TuxMath saves summaries of the last ten played games in the players .tuxmath
directory (the same place the options file is stored). These files, which are
named "summary1" through to "summary10" include list of asked and missed
questions along with numbers of correct and incorrect answers and more
information. 'summary1' is the most recent game.

In addition to this, there is a 'log.csv' that contains a one line summary of
each attempted misson. The player must have answered at least one question for
it to count as an attempt. This file can be imported into a spreadsheet program
to examine or chart progress.
