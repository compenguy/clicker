# clicker
Linux+X Windows automatic keyboard and mouse clicker

`clicker` waits in a loop until `numlock` is active, at which point it will repeatedly press the specified key(s) at the specified interval(s) until suspended (by toggling `numlock`) or terminated (`<ctrl+c>` will do).

# usage
Running clicker with no additional options causes it to click the primary mouse button ~20/sec while `numlock` is enabled.

To disable mouse clicking entirely, pass the `-c 0` option.

```Usage:
  clicker [OPTION?] - sends input signals to currently active window in an X session

Help Options:
  -h, --help                      Show help options

Application Options:
  -x, --x11-display=NAME          The X11 display to send the input to. Default: DISPLAY env var
  -d, --delay=N                   Delay in msecs before sending any input events. Default: 250
  -m, --mouseclick=N              Click mouse button N at regular intervals. Default: 1
  -c, --mouseclick-interval=N     Delay in msecs between mouse button clicks. Default: 50, 0: disable
  -k, --keypress=KEY              Press key with the X11 name of KEY at regular intervals. Default: none
  -p, --keypress-interval=N       Delay in msecs between key presses. Default: 1000, 0: disable
```

# how to build
Prerequisites: 
* gcc
* pkg-config
* the following pkg-config packages:
   * x11 (libx11-dev)
   * xtst (libxtst-dev)
   * xext (libxext-dev)
   * glib-2.0 (libglib2.0-dev)

Run `make clicker.release`

Copy `clicker.release` anywhere you want - `~/`, `/usr/local/bin`, etc.  You'll probably want to rename it as just `clicker`, though.
