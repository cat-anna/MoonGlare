
## Engine configuration accessible from scripts


## Basic runtime configuration

Structure can be accessed via `require "RuntimeConfiguration"` only from init.lua scripts.

Example:
```lua
local rtconf = require "RuntimeConfiguration"
rtconf.scene.firstScene = "MainMenu"
rtconf.scene.loadingScene = "LoadScene"
rtconf.consoleFont = "Arial"
```
