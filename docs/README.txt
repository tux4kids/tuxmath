"Tux, of Math Command"
An educational math tutorial game starring Tux, the Linux Penguin
-----------------------------------------------------------------

May 18, 2006


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
    [ UNDER CONSTRUCTION ]  Just double-click the "tuxmath.exe" icon. ???
    To be prompted for command line options (see below), invoke the game
    from the "Run" dialog, which you can get by selecting the "Run Program"
    item from Windows' "Start" menu. [ IS THIS CORRECT? ]

  MacOS
  -----
    [ UNDER CONSTRUCTION ]  Just double-click the "tuxmath" icon. ???
    To be prompted for command line options (see below), hold the [OPTION] key
    as you double-click the icon.


  Command Line Options
  --------------------
    The following command-line options can be sent to the program.

     --norepeats      -  Game consists of working through a list of questions
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

      --nosound       -  Do not play any sounds or music.
      -s
      --quiet
      -q

      --nobackground  -  Do not display photographic backgrounds in game.
      -b                 (Useful on slower systems.)

      --keypad        -  Display an on-screen numeric keypad.  (Useful
      -k                 for touch screens or in place of a physical keyboard.)

      --operator OP   -  Add an operator to the game (will cause the program
      -o OP              to ignore saved option screen settings).  You can
                         use this switch multiple times to run the game
                         with multiple operators.

                         Valid values for "OP" are:

                           add
                           subtract
                           multiply
                           divide

      --demo          -  Demo mode.  The game will cycle back and forth
      -d                 between the title and the game, and it will
                         auto-play the game.  The only user interaction
                         can be for quitting or pausing.

      --allownegatives   Allows subtraction answers to be less than zero.
      -n                 When selected, the led numbers at the top of the
                         screen will include a fourth digit for the '-' sign.
                         Also, if --keypad is selected, the '-' and '+' may
                         be grayed-out depending if negatives are allowed.


    These command-line options display useful information, but the program
    does not attempt to start up in interactive mode.

      --help          -  Display a short help message, explaining how to
      -h                 play the game.

      --usage         -  Display the available command-line options.
      -u

      --version       -  Display the version of "tuxmath" you're running.
      -v

      --copyright     -  Display copyright information
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
    By default, the game operates in an arcade-style manner, continuing
    until you lose all of your cities.  A GAME OVER screen is displayed.
    By pressing any key or clicking the mouse, you return to the title
    screen.

    It is now possible to play through a defined list of questions. This
    mode is selected via the "--norepeats" command line argument. By
    default, the questions are asked in a random order.  If answered
    correctly, they are removed.  A question that is not answered correctly
    (allowing the comet to destroy its target) will reappear in random
    order.  If all questions are successfully answered before the cities
    have been destroyed, the player wins and a "victory" screen is displayed.


  Regaining Cities
  ----------------
    [ Rules will go here ]


  Advancing Waves
  ---------------



Setting Game Options
--------------------
  [ UNDER CONSTRUCTION ]
  This is still under construction, but many things can be set.  For now,
  there are three ways to set game options.

  1. Many command-line options are supported (see above).

  2. The "Options" screen allows several parameters to be set at run-time,
  or reset between individual games while the program is still running.
  Currently supported settings include the math operations to be used for
  questions, the starting speed, the maximum value of answers (for division
  questions, this is the maximum size of the dividend, not actually the 
  answer), and ranges of numbers to be used to generate questions.

  3. Editing the default values in tuxmath.h and mathcards.h and recompiling.
  The default settings for general game options are contained in tuxmath.h, and
  the defaults for math question settings are in mathcards.h.  Very fine-grained
  control over game behavior is offered, but this isn't exactly a user-friendly
  method of controlling the program.

  Two main improvements are planned.  First, the program should read
  and write the settings to disk in a human-readable fashion, where they
  could be modified with a text editor.  This also would allow creation of
  a series of "lessons" that could be played in a planned order.
  Second, the "Options" screen needs to be overhauled to give access to all
  settings from within the program.


Setting Administrative Options
------------------------------
  "Tux, of Math Command" allows parents/teachers to adjust which parts
  of the game options can be changed by the player.

  For example, if you wish to, you can completely lock out all
  "Division" questions.  The students/children will still be able to enable
  and disable "Addition," "Subtraction," and "Multiplication" as they
  wish.

  On the other hand, you may wish to lock-in the other three kinds
  of equations, so that the players cannot disable any of them.
  All games will always have addition, subtraction and multiplication
  problems, but will never have division problems.

  [ UNDER CONSTRUCTION ]



License
-------
  "Tux, of Math Command" is Free Software, distributable under the
  GNU General Public License (GPL).  Please read COPYING.txt for more info.



Credits
-------
  Designed by Sam "Criswell" Hart  <criswell@geekcomix.com>
  Software by Bill Kendrick  <bill@newbreedsoftware.com>

  Please see the game's "Credits" screen for a complete list of contributors.



Software Used
-------------
  GNU C Compiler
    http://www.gnu.org/

  The GIMP
    http://www.gimp.org/

  KDevelop
    http://www.kdevelop.org/
