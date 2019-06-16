# game-launcher

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
