# Koi Examples

This collection aims to show the usage of all convenience features.

To execute the example, Koi must be built and installed, either system-wide or locally. The Makefile in each directory assumes a local installation.


## Capture the Flags and CrackMes

### HexRays CTF 2023

The [HexRays CTF 2023 Challenge](https://hex-rays.com/blog/free-madame-de-maintenon-ctf-challenge) is a great example of when satisfiable module theory (SMT) must be used to solve path constraints. The natural next step is to automate the solving process through the use of symbolic execution. In this example, you will find:

- Custom function hook for `strncpy`.
- Instruction hooks to break from loops early.
- Manually constraining memory to be easily typable.


### Key Generation

Where there is key generation, symbolic execution stands as a tool to generate keys. In this [crackmes.one sample](https://crackmes.one/crackme/6784563e4d850ac5f7dc5137) you will find:

- Baited function hook for `strcpy`.
- Instruction hooks to spoof the length of a string.
- Several jump condition injections.
- Several dead end definitions.
- The generation of multiple solutions.


### Simple strcmp Injection

This [crackmes.one sample](https://crackmes.one/crackme/6784f8a84d850ac5f7dc5173) can easily be solved in less than a minute using a tool like Ghidra by even a novice reverse engineer. As such, it is a poor use case for symbolic execution, but proves to be an easy to understand example for:

- Function hooks for user input functions.
- Function hooks for printing functions.
- A simple jump condition injection based on `strcmp`.
- How to name and symbolize memory.
- Nifty output, since there are three secrets to unviel.


### Simple XOR

Not all binaries need to be explored to an exit point, as proven in this [crackmes.one sample](https://crackmes.one/crackme/6715466c9b533b4c22bd18bb). Here, the expected input is stored in memory before user input, allowing it to be extracted early on. In this example, you will see:

- Baited function hook for `strlen`.
- Instruction hook to dump function parameter information.
- Instruction hook to extract a key from memory.


## Memory Violations

### Double Free

This example is possibly the most trivial, where a call to `free` is performed two times for the same pointer. In this very simple example, you will find:

- Custom function hook for `free` to detect a double free.
- Baited hooks for `malloc` and `strchr`.


### Invalid Pointer

This example showcases the consequences of pointer arithmetic in relation to how memory is free'd. Namely the fact that any memory used in a call to `free` must be the return value of `malloc`, `calloc`, or `realloc`; not an different address within the allocated memory space. More specifically, you will see:

- Custom function hook for `free` to detect an invalid pointer.
- Baited hooks for `calloc` and `free`.
- Value of the invalid pointer and what should be used instead.


### String Overflow

This examples tracks the state of memory is to identify various buffer overflows that can arise from copying a string. More specifically, you will find:

- Custom function hook for `strcpy` to comprehensively detect buffer overflows.
- Baited function hooks for `fgets` and `strlen` on concrete and symbolic memory.
- Buffer overflows from stack to stack, heap to heap, and stack to heap.
- Identify extreme stack-buffer overflows that lead to stack smashing.
- The analysis of four intentionally vulnerable executables.


### Use After Free

This example analyzes a rudimentary program written by ChatGPT to create, read, and modify files. Unfortunately, it includes a use-after-free weakness, even without asking for it in the prompt. So, maybe consider that next time it spits out some code. Here, you will see:

- Custom function hook for `fprintf` to detect use after free.
- Baited hooks for `malloc` and `free`.
- Symbolic exploration to the use after free and a necessary input to reach it.
- Detailed output on where the buffer was allocated, free'd, and later used.


## Exploitations

### Format String Vulnerabilty

This example demonstrates my favorite exploitable vulnerability. When a format string is controlled by user input (i.e. is symbolic), arbitrary memory can be read and written to. This can lead to all sorts of nasty things, but this [crackmes.one sample](https://www.crackmes.one/crackme/66feb83e9b533b4c22bd0c13) demonstrates how a global variable can be modified to pass an otherwise impassible guard. This example shows:

- Custom function hook for `printf` to validate the format string.
- Handling of 32-bit calling convention using the stack.
