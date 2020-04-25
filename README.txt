This is a experimental small C-like compiler to lean compilation principles.

1. How to build
  a. cd compiler
  b. make
There will be a executable binary generated, named "compiler"

2. How to compile a c file
  ./compiler ../test/test.c
  Above compilation will generate two files, one is intermediate representation(test.ir), another is asm file(test.s)

3. There are still many todo tasks, like IR optimization, assambler and linker.
