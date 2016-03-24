## Infos

**Author**: awe  
**Category**: Misc  
**Difficulty**: Medium  

## Description

Survive the robot uprising and steal their secret flag!  
Service running on `robots.insomni.hack:4242`  

## Solution:

Teleport close to an edge, get out of the game board a number of times so robots will get aligned.  
Teleport again so you're aligned with the robots and there's a grave in between. They'll all collide with it and you'll win the game.  

Read files `/proc/self/cmdline` to notice that it's running with lisp and that the script is named `robots.lisp`  
Read `/proc/self/cwd/robots.lisp` to leak the source code.  

The `(read)` function in lisp is actually a `READ-EVAL-PRINT`, so you can make it execute arbitrary lisp code and thus get a shell. (gg lisp)  

Exploit:
```lisp
#.(ext:shell "cd /home/robots ; find ; cd getflag ; ./read_flag ; id")
```

The source links to the original source code, where the vulnerablity was present already ;)
