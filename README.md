qsdl2 (Experimental!)
=====================

An experiment in binding SDL2, for kdb+/q.

Ever wanted to write a graphical app or game in q? Now you can!

# BIG FAT WARNING!

This is *experimental*, there are no guarantees about stability or data.

For example, I have crashed it with large numbers of points/lines as a
parameter to a draw call because of how SDL uses stack allocation (`alloca()`) internally.


# Build

    $ make {m32,m64,l32,l64}
    $ cp qsdl2_{m32,m64,l32,l64}.so /path/to/q/bin

# Use

    q) \l sdl2.q

Take a look at `example.q` for a rough idea of "real" usage.

NOTE: You might need to set `DYLD_LIBRARY_PATH` or `LD_LIBRARY_PATH` environment variables
(Mac and Linux respectively) to the directory where the `.so` lives before running `q`.

# Licence

LGPLv3. See `LICENSE` and `COPYING.LESSER`.

Copyright (c) 2017 Lucas Martin-King.

Other parts of this software (eg: SDL2) are covered by other licences.
