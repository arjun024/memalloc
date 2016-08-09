# memalloc

memalloc is a simple memory allocator.

It implements <a href="http://man7.org/linux/man-pages/man3/free.3.html">malloc()</a>, <a href="http://man7.org/linux/man-pages/man3/free.3.html">calloc()</a>, <a href="http://man7.org/linux/man-pages/man3/free.3.html">realloc()</a> and <a href="http://man7.org/linux/man-pages/man3/free.3.html">free()</a>.

####Article####

Write a simple memory allocator

(http://arjunsreedharan.org/post/148675821737/write-a-simple-memory-allocator)

####Compile and Run####
```
gcc -o memalloc.so -fPIC -shared memalloc.c
```

The `-fPIC` and `-shared` options makes sure the compiled output has position-independent code and tells the linker to produce a shared object suitable for dynamic linking.

On Linux, if you set the enivornment variable `LD_PRELOAD` to the path of a shared object, that file will be loaded before any other library. We could use this trick to load our compiled library file first, so that the later commands run in the shell will use our malloc(), free(), calloc() and realloc().

```
export LD_PRELOAD=$PWD/memalloc.so
```

Now, if you run something like `ls`, it will use our memory allocator.
```
ls
memalloc.c		memalloc.so
```

You can also run your own programs with this memory allocator.
