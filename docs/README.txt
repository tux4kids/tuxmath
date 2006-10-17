"Tux, of Math Command"
An educational math tutorial game starring Tux, the Linux Penguin
-----------------------------------------------------------------

Oct 17, 2006

For tuxmath-0.98

Objective
---------
  In "Tux, of Math Command," you play the part of Commander Tux, as he
  defends his cities from an attack of math equations.

  Comets are crashing towards your cities, and you must destroy
  them by solving their equations.



Installation
------------
  For instructions on installing the game on your system,
  please read the "INSTALL.txt" file.



Running The Program
-------------------
  Linux/Unix
  ----------
    Simply type the command "tuxmath" at a command prompt (eg, in an xterm).

    Depending on your graphical interface or window manager, you can
    probably also create a clickable icon which will launch the game.
    See your interface's documentation or help screens for details.

  Windows
  -------
    Just double-click the "tuxmath.exe" icon or select "Tuxmath" in the Start
    Menu.  The current installer creates menu items to run tuxmath either in
    Fullscreen mode or within a 640 x 480 window.
    To be prompted for command line options, run tuxmath from the "Run" dialog
    or the "C:> Command Prompt" console. Type "tuxmath.exe" followed by any 
    desired options (see below). 

  MacOS
  -----
    [ UNDER CONSTRUCTION ]  Just double-click the "tuxmath" icon. ???
    To be prompted for command line options (see below), hold the [OPTION] key
    as you double-click the icon.


  Command Line Options
  --------------------
    NOTE: editing the config file is now a much better way to control the 
    behavior of Tuxmath - SEE BELOW.  There is also a simple GUI-based
    config program packaged with the Windows build.

    The following command-line options can be sent to the program.
     --optionfile filename - play game based on settings in the named file (see
                         below for more on tuxmath config files). Tuxmath will
                         look for a valid config file with a matching name in
                         the following locations:
                         	1. current working directory
				2. as an absolute pathname
				3. in the missions directory with tuxmath's 
                                   other data files.
				4. in the user's tuxmath options directory
                                   (e.g. /home/laura/.tuxmath/filename
                                5. in the user's home directory.

     --playthroughlist - Game consists of working through a list of questions
      -r                 generated based on the selected options (or defaults).
                         If a comet strikes a city without being shot down by
                         the player, the question is reinserted into the list
                         in a random location. If the player answers all questions
                         correctly before the cities are destroyed, he/she wins.
                         If all cities get destroyed, the game ends in defeat.

     --answersfirst   -  to ask questions in format: ? + num2 = num3 instead of 
                         default format: num1 + num2 = ?.

     --answersmiddle  -  to ask questions in format: num1 + ? = num3 instead of
                         default format: num1 + num2 = ?.

     --fullscreen     -  Run the game in full screen, instead of in a window,
      -f                 if possible.

     --windowed       -  Run the game in a 640 x 480 window.
      -w

     --nosound        -  Do not play any sounds or music.
      -s
     --quiet
      -q

     --nobackground   -  Do not display photographic backgrounds in game.
      -b                 (Useful on slower systems.)

     --keypad         -  Display an on-screen numeric keypad.  (Useful
      -k                 for touch screens or in place of a physical keyboard.)

     --operator OP    -  Add an operator to the game (will cause the program
      -o OP              to ignore saved option screen settings).  You can
                         use this switch multiple times to run the game
                         with multiple operators.

                         Valid values for "OP" are:

                           add
                           subtract
                           multiply
                           divide

     --demo           -  Demo mode.  The game will cycle back and forth
      -d                 between the title and the game, and it will
                         auto-play the game.  The only user interaction
                         can be for quitting or pausing.

     --allownegatives    Allows subtraction answers to be less than zero.
      -n                 When selected, the led numbers at the top of the
                         screen will include a fourth digit for the '-' sign.
                         Also, if --keypad is selected, the '-' and '+' may
                         be grayed-out depending if negatives are allowed.


    These command-line options display useful information, but the program
    does not attempt to start up in interactive mode.

     --help           -  Display a short help message, explaining how to
      -h                 play the game.

     --usage          -  Display the available command-line options.
      -u

     --version        -  Display the version of "tuxmath" you're running.
      -v

     --copyright      -  Display copyright information
      -c



Program Navigation
------------------
  Title Screen
  ------------
    On this screen, you can choose to play the game, go to the Options
    Screen, view the credits, or quit.

    Use the [UP] and [DOWN] arrow keys to select what you wish to do,
    and then press [ENTER / RETURN / SPACEBAR].  Or, use the mouse to click the
    menu item.

    Pressing [ESCAPE] will quit the program.


  Options Screen
  --------------
    On this screen, you can select some of the gameplay options or return to 
    the Title Screen.  Currently, the four math operations can be enabled
    or disabled, as well as the speed setting and ranges of numbers to use. 

    Use the [UP] and [DOWN] arrow keys to select what you wish to do,
    and then press [ENTER / RETURN /SPACEBAR].  Or, use the mouse to click the
    menu item.

    Mouse support has been added.

    Pressing [ESCAPE] will return to the Title Screen.  Currently, there is no
    method of doing this with the mouse.


  Credits Screen
  --------------
    This screen displays the credits.  You can press [ESCAPE] to return
    to the title screen.



How To Play
-----------
  Destroying Comets
  -----------------
    As the comets fall towards your cities, you must solve their equations.

    To destroy it:
    --------------
      First, figure out the answer to the equation.
      For example, "3 x 4 = ?" would be "12"

      Second, type in the answer.  As you type numbers on the keyboard, they
      will appear in the "LED"-style display at the top center of the screen.
      If negative answers are enabled, there will be a fourth place in the
      LED display for the minus sign.  The '-' and '+' keys will toggle the
      minus sign on and off, respectively.

      Finally, press [ENTER / RETURN].


    The comet that has the number you entered as its answer will
    be shot down by Tux the penguin.

    Note: Sometimes more than one comet will have the same answer.
          In this case, the comet closest to your cities will be
          destroyed first.  [Perhaps all should be destroyed?]

    Note: After typing [ENTER / RETURN], the "LED"-style display will
          automatically reset to "000" for you, so you can answer the
          next equation!


  Correcting Your Answer
  ----------------------
    If you made a mistake as you typed in your answer, you can press
    [BACKSPACE / DELETE] and the "LED"-style display at the top center
    of the screen will reset to "000".


  Using the On-Screen Keypad
  --------------------------
    If you launched the program with the "--keypad" (or "-k") option,
    the game screen will also have an 11-key numeric keypad on the
    center of the screen.  (It has a similar layout to most keyboard
    number pads and calculators.)

    Using the mouse pointer to click on the on-screen buttons acts
    just like typing numbers on the keyboard.

    This feature could be useful for computers with touchscreens,
    or for players who cannot use a keyboard.


  Losing A City
  -------------
    If a comet crashes into one of your cities before you had the
    chance to answer its equation, the city's shields will be
    destroyed.  If the city is hit by another comet, it will be
    completely destroyed.

  Ending The Game
  ---------------

    The default mode is now to play through a defined list of questions. This
    mode is selected by setting the config file 'play_through_list' parameter
    to 1 ('yes' or 'true'), or via the "--playthroughlist" command line argument. 
    By default, the questions are asked in a random order.  If answered
    correctly, they are removed.  A question that is not answered correctly
    (allowing the comet to destroy its target) will reappear in random
    order.  If all questions are successfully answered before the cities
    have been destroyed, the player wins and a "victory" screen is displayed.

    The older arcade-style mode is also supported, in which the game continues
    until you lose all of your cities.  A GAME OVER screen is displayed.
    By pressing any key or clicking the mouse, you return to the title
    screen.

  Regaining Cities
  ----------------
    [ Under construction ] Briefly, there will be special "bonus comets" at
    certain intervals that will have the effect of either activating shields or
    rebuilding cities if answered correctly.


  Advancing Waves
  ---------------



Setting Game Options
--------------------
  [ UNDER CONSTRUCTION ]

  1. The program now reads and writes the settings to disk in a human-readable
  fashion, where they can be modified with a text editor. The file is created
  in the user's home directory within a directory called ".tuxmath" and is 
  simply called "options". As an example, a user "laura" on a Unix/Linux system
  would find this at /home/laura/.tuxmath/options.  The file contains
  extensive comments describing all settings. By editing and saving this file,
  very extensive control over the program is supported, particularly with
  respect to generation of math questions. There really is no need to use
  command-line options any more. In the near future, I plan to
  include a series of "lessons" that could be played in a planned order.

  2. Many command-line options are supported (see above). 

  3. The "Options" screen allows several parameters to be set at run-time,
  or reset between individual games while the program is still running.
  Currently supported settings include the math operations to be used for
  questions, the starting speed, the maximum value of answers (for division
  questions, this is the maximum size of the dividend, not actually the 
  answer), and ranges of numbers to be used to generate questions. However,
  many settings are only selectable via the config file.  This will be addressed
  in a later version of the program.

  3. Editing the default values in tuxmath.h and mathcards.h and recompiling.
  Now that the program reads and writes the settings from/to disc, this is of
  interest only to developers.


Setting Administrative Options
------------------------------
  "Tux, of Math Command" allows parents/teachers to adjust which parts
  of the game options can be changed by the player.

  The game options are first read from a master config file in the program's
  data directory (/usr/local/share/tuxmath/.tuxmath on *nix if installed 
  using "make install"), then overridden by the user's own .tuxmath config
  file if "per_user_config" is selected in the master .tuxmath config file.
  If "per_user_config" is deselected, the game starts up with the master
  settings.  (This is somewhat under construction).


Game Summary Files
------------------
  "Tux, of Math Command" saves summaries of the last ten games in the player's
  .tuxmath directory.  The files are named "summary1" through "summary10",
  with "summary1" the most recent. The files includes lists of questions asked
  and questions missed, along with the numbers of correct and incorrect
  answers and the percent correct.

  [ UNDER CONSTRUCTION ]



License
-------
  "Tux, of Math Command" is Free Software, distributable under the
  GNU General Public License (GPL).  Please read COPYING.txt for more info.



Credits
-------
  Designed by Sam "Criswell" Hart  <criswell@geekcomix.com>
  Software by Bill Kendrick  <bill@newbreedsoftware.com>

  Current maintainer/programmer David Bruce <dbruce@tampabay.rr.com>

  Please see the game's "Credits" screen for a complete list of contributors.



Software Used
-------------
  GNU C Compiler
    http://www.gnu.org/

  The GIMP
    http://www.gimp.org/

  KDevelop
    http://www.kdevelop.org/
