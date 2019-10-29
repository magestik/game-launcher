# game-launcher

[![Build Status](https://travis-ci.org/magestik/game-launcher.svg?branch=master)](https://travis-ci.org/magestik/game-launcher) [![Build status](https://ci.appveyor.com/api/projects/status/ykpj63fafad3yaje/branch/master?svg=true)](https://ci.appveyor.com/project/magestik/game-launcher/branch/master)

## Compilation

<pre>
mkdir build
cd build
cmake ..
cmake --build .
</pre>

## Dependencies

* libjansson
* xcb or Xlib (this static dependency will be removed in a future release to be loaded dynamically at runtime, if available)
* cairo
