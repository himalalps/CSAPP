# Tips on how to use GDB

## Interface

First, use command below to start GDB:

```bash
$ gdb ./bomb
```

Then you will see the following interface:

```bash
GNU gdb (Ubuntu 12.1-0ubuntu1~22.04) 12.1
Copyright (C) 2022 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "x86_64-linux-gnu".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
--Type <RET> for more, q to quit, c to continue without paging--
<https://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from ./bomb...
```

This is very plain, we can use command below to show the assmbly or registers:

```bash
(gdb) layout asm
(gdb) layout regs
```

## Breakpoints

We can use command (below two are the same for `bomb`) to set breakpoints at specific address or function:

```bash
(gdb) break *0x400da0
(gdb) break main
```

We can use command below to show all breakpoints:

```bash
(gdb) info breakpoints
```

## Run

We can use command below to run the program:

```bash
(gdb) run
```

## Step

We can use command below to step into the next instruction:

```bash
(gdb) stepi
```

If we want to step over a function call, we can use command below:

```bash
(gdb) nexti
```

## Memory

We can use command below to show the memory of register or specific memory:

```bash
(gdb) x/[NUM][SIZE][FORMAT] where
(gdb) x/100hx $rsp
(gdb) x/20bx 0x7ffffffedf00
```

SIZE stands for size of each object (b=byte, h=half-word, w=word, g=giant (quad-word)).

FORMAT is how to display each object (d=decimal, x=hex, o=octal, etc.).

We can use command below to output the memory to a file:

```bash
(gdb) dump memory [FILENAME] [START] [END]
```

## Registers

Besides the `layout` command, we can also use command below to show the registers:

```bash
(gdb) info registers
```