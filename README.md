# Hazuki

> Let's reinvent the wheel, but make it square!

Hazuki is a lightweight, portable C library which provides
some basic data structures and algorithms. It is very immature
and is **not** meant to be used in production code! Use at your
own risk!

Core development principles:

- Compiles cleanly as C99
- No external dependencies
- No platform-specific code
- No global state
- Minimize undefined behavior

Note that unlike many C libraries, Hazuki is unforgiving of
invalid arguments - violating function contracts will result
in the immediate termination of your program using `abort()`.
This is necessary to maintain a simple API without excessive
return value checking.

Named after [Hazuki Tachibana](https://vndb.org/c20285).

## Contents

- `vector.h`: Self-resizing array (a.k.a. `std::vector` in C++)
- `map.h`: Key-value store (a.k.a. `std::unordered_map` in C++)
- `utils.h`: Common utility functions

## License

Distributed under the [MIT License](http://opensource.org/licenses/MIT).
