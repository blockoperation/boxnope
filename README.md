# boxnope

A simple X11 session/launcher menu. I just threw it together quickly so I could
use KWin without Plasma. Think of it as a *box WM without the WM itself.


## Features

  * Wallpaper management
  * Launcher menu
  * Spawns and kills a WM
  * Spawns an autostart script


## Requirements

  * Qt 5.x (I'm using 5.6, but older/newer versions may work)
    * Core
    * Widgets
    * X11Extras
    * XmlPatterns
  * KGlobalAccel (for keyboard shortcuts, optional)
  * libxcb


## TODO

  * Desktop switching menu
  * Ability to reconfigure/restart a running instance
  * Support for multiple screens
  * More wallpaper options
    * More scaling options (e.g. center, tile, max with aspect retained, etc)
    * Different wallpaper per screen
    * Span a single wallpaper across multiple screens
  * Documentation!
