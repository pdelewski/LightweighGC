# SafeReference

This repository contains implementation of SafeReference abstraction that is
yet another attempt to safe memory programming. Of course this is one
of ingredients of safe programming and currently it does not resolve 
problems like array indexing and so on.

It's something between GC (garbage collection) and safety quarantees provided
during compilation time like in Rust. It does not introduce such friction
and steep learning curve like borrow checker.

Safety rules are checked during runtime, however they are pretty lightweight.

One of the most important guarantee is that your program can crash during runtime,
but in controlled way which means immediate crash in the case when one of the below
rules are not fulfilled.

## List of rules:

- There is always one owner of resource at any point in time
- Each alias has to be released (set to null) before its destructor is called