prompt-sort
===========
Sort a file interactively.

**prompt-sort** [**-n**] [**-t** *count*] [*file*]

Sorts lines in *file* (or standard input if not given) by presenting a
series of choices to the user. The prompts are output on standard error
so that **prompt-sort** may be used as part of a pipe, from an editor,
etc.

Pass **-n** to number the output.

Pass **-t** *count* to limit the ranking to the top *count* items. This
is more efficient than using [head](https://man.openbsd.org/head.1) on
the output as it reduces the number of choices that must be made.

Example
-------
Raking a top-3 of games:

    $ prompt-sort -t3 -n games.txt
      1. ICO
      2. Zelda: The Minish Cap
    choice? 1
    
      1. Zelda: The Minish Cap
      2. Shadow of the Colossus
    choice? 2
    ...
    
      1. Death Stranding
      2. The Last of Us Part II
      3. Kirby's Return to Dream Land

Compiling
---------
Should work on Linux, BSDs, macOS, etc:

    make
    sudo make install

Author
------
Sijmen J. Mulder (<ik@sjmulder.nl>). See [LICENSE.md](LICENSE.md).
