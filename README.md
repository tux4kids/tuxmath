# Tux, of Math Command ("TuxMath")

## Description

TuxMath is an educational arcade game starring Tux, the Linux Penguin! Players help Tux defend his igloos (or cities in the classic mode) by solving math problems. Comets fall, each carrying a math question. Answer correctly, and Tux's laser zaps the comet. Miss, or be too slow, and the comet will damage an igloo.

It's designed to be fun and to help kids practice their math facts and develop numerical fluency.

## Main Game Modes

*   **Comets:** The classic game mode. Math problems (addition, subtraction, multiplication, division) fall as comets. Answer them before they hit Tux's igloos! This mode includes various training levels from simple number typing up to complex arithmetic involving all four operations and negative numbers.
*   **Factoroids:** Pilot a spaceship through an asteroid field. Each asteroid has a number. Shoot it with the correct prime factor to break it down! This mode helps practice prime factorization.
*   **Fractions (Variant of Factoroids):** Similar to Factoroids, but focused on understanding and simplifying fractions.

## Key Features

*   **Multiple Levels:** From simple number typing in "Math Command Training Academy" to challenging arithmetic in "Arcade" modes.
*   **Variety of Operations:** Covers addition, subtraction, multiplication, and division.
*   **LAN Multiplayer Mode:** Compete with others on your local network in Comets mode.
*   **Factorization Practice:** Unique "Factoroids" game for learning prime factors.
*   **Customizable Question Lists:** Advanced users and educators can create custom mission files.
*   **Theming Support (New!):** The game's appearance can be changed using themes. Create your own or use themes provided! (e.g., a new "Egyptian" theme).
*   **SVG Image Support (New!):** Some game graphics are now using Scalable Vector Graphics for crisp visuals at any resolution.
*   **End-of-Game Review (New!):** After a game session in Comets or Factoroids, players can review the questions they answered incorrectly.
*   **Play Again with Missed Questions (New for Comets!):** In Comets mode, after reviewing incorrect answers, players have the option to start a new game session populated exclusively with those questions they previously missed.

## Compiling and Running

### General Dependencies

You'll need SDL (Simple DirectMedia Layer) and its associated libraries:
*   SDL
*   SDL_image (for PNG, JPG loading)
*   SDL_mixer (for sound and music)
*   SDL_ttf (for font rendering, though often handled by SDL_Pango now)
*   SDL_Pango (for advanced text rendering)
*   SDL_gfx (for some graphical primitives, though NanoSVG is now used for SVGs)
*   (Potentially) librsvg or NanoSVG - NanoSVG is now included directly for SVG support.
*   A C compiler (like GCC) and standard build tools (make, autotools, cmake).

### Compiling with Autotools (Linux/macOS typical)

1.  If you're compiling from a Git repository for the first time, you might need to generate the `configure` script:
    ```bash
    autoreconf -ivf
    ```
2.  Run the configure script. You might need to specify a prefix if you don't want to install to `/usr/local`:
    ```bash
    ./configure --prefix=/your/install/path
    ```
    (For development, `./configure` is often enough).
3.  Compile the game:
    ```bash
    make
    ```
4.  Install (optional, you can often run from the `src` directory):
    ```bash
    sudo make install
    ```

### Compiling with CMake (Alternative)

CMake can also be used. From the top-level directory:
1.  Create a build directory:
    ```bash
    mkdir build && cd build
    ```
2.  Run CMake:
    ```bash
    cmake .. -DCMAKE_INSTALL_PREFIX=/your/install/path
    ```
3.  Compile:
    ```bash
    make
    ```
4.  Install (optional):
    ```bash
    sudo make install
    ```

### Running the Game

After installation, you should be able to run `tuxmath` from your terminal.
If you haven't installed it, you can usually run it from the `src` directory after compiling:
```bash
cd src
./tuxmath
```

Please refer to the `INSTALL` file for more detailed compilation instructions and platform-specific notes.

## Configuration

TuxMath uses configuration files to control game options, math problem parameters, and more.
*   Global settings are typically in `[DATA_PREFIX]/missions/options`.
*   User-specific settings are stored in `~/.tuxmath/options` (on Linux/macOS) or in your user's Application Data directory (on Windows).
*   The new **theming system** reads the `CURRENT_THEME` setting from these configuration files. For example, to use the "egyptian" theme, you would set `CURRENT_THEME = egyptian` in your options file.

## Other Documentation

*   **`INSTALL`**: Detailed installation instructions.
*   **`doc/TODO`**: A list of features, enhancements, and bugs that developers are aware of or plan to work on.
*   **`doc/changelog`**: A history of changes made to the game.
*   **`doc/html user guide/`**: An HTML user guide (may require building or opening `index.html`).
*   **`COPYING`**: Contains the GNU General Public License under which TuxMath is distributed.

## Contributing

We welcome contributions to TuxMath! Here are a few ways you can help:

*   **Bug Reports:** If you find a bug, please check the existing `doc/TODO` and if it's not listed, consider reporting it. Information on how to report bugs can usually be found on the project's website or development portal (e.g., SourceForge, GitHub, or the Tux4Kids Alioth Debian project page mentioned in older files).
*   **Feature Requests:** Have a great idea? Let the developers know!
*   **Translations:** TuxMath supports multiple languages. You can help translate it into your language.
*   **Themes:** With the new theming system, creating and sharing new themes is a great way to contribute.
*   **Code Contributions:** If you're a developer, patches and code contributions are welcome. It's usually best to discuss significant changes with the developers first.

Thank you for playing and supporting TuxMath!
