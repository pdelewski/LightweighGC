# SafeReference

This repository contains implementation of SafeReference abstraction that is
yet another attempt to safe memory programming.

It's something between GC (garbage collection) and safety quarantees provided
during compilation time like in Rust. It does not introduce such friction
and steep learning curve like borrow checker.

Safety rules are checked during runtime, however they are pretty lightweight.

## List of rules:

- There is always one owner of resource at any point in time
- Each alias has to be released (set to null) before its destructor is called