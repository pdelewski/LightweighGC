# SafeReference

This repository contains implementation of SafeReference abstraction that is
yet another attempt to safe memory programming.

It's safety rules are checked during runtime.

## List of rules:

- There is always one owner of resource at any point in time
- Each alias has to be released before (set to null) before its destructor is called