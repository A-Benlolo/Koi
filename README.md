# Koi

<p align="center"> <img src="https://github.com/A-Benlolo/Koi/blob/main/doc/img/koi_logo.svg?raw=true"/> </p>

**Koi** is a **C++** extension to the [Triton](https://github.com/JonathanSalwan/Triton) dynamic binary analysis library that provides convenience features for **symbolic execution**. Existing convenience features include:

- Automatic **mapping of x86/64 ELF files**.
- Instruction and function **hooking**, including a growing list of **libc hooks**.
- Instruction and jump condition **injection**.
- Defining of **exploration limits**...
    - Limit on maximum fork **depth.**
    - Limit on maximum repeated instruction **execution**.
    - Stop exploring at defined **dead ends**.
- Dynamic **tracking of the heap and stack**.





## Terminology


### Swimmer

**Koi** utilizes a **Swimmer** to perform symbolic exploration on a memory pool from a defined source to a defined target. All **satisfiable** paths of code are explored until a maximum depth has been reached or one instruction has been executed too many times. The constraints to follow each **branch point** is tracked so that the swimmer can provide a **model** on how to reach the target.


### Hooks

While exploring, **instruction hooks** can be used to execute your code *after* the instruction has been executed. This allows for inspection and modification of the Swimmer state at that time.

Alternatively, **function hooks** will override calls in the binary code to *execute your code instead*. This is ideal for implementing library functions or skipping of functions and stubbing known side effects.


### Bait

To alleviate some workload, **pre-developed function hooks**--called **bait**--are provided. These are intended to perform the original purpose of the function, not perform any special checks. Therefore, it is most beneficial to call on bait functions **after performing your own hook**. For example, checking if `strcpy()` results in an overflow before performing the copy. For an example of this see [Memory Violations](./examples/Memory_Violations).


### Injection

To override an instruction, **instruction injection** can be used. This executes the new instruction and *ignores the original instruction*. This is a way to test patching without commiting it to a binary file.

A varient of instruction injection that goes beyond patching is **jump condition injection**. This allows for a more robust model to be created when **for branch points that are constrained by the return value of a function**. This is a powerful but difficult to grapple functionality, so see [Simple_strcmp_Injection](./examples/Simple_strcmp_Injection) for an example.


### Dead End

Sometimes a path exists that is satisfiable, but is of no interest. In this case, paths that reach this known address can be **killed** to speed up exploration.





## Docker

A Dockerfile is provided for an environment with Koi installed (including header files) to explore the examples in this repository. That means there's no worry about building or installing. Have fun!





## Building Koi

### Prerequisites

To build Koi, you must first install [Triton](https://github.com/JonathanSalwan/Triton) and the header files for LibElf.


### System Installation

This is the path of least resistance, but requires you trust the security of this codebase:

1. Execute `sudo make install`
    - Compiles a release-varient of the shared object
    - Copies the shared object to `/usr/local/lib`.


### Local Installation

If you cannot (or do not want to) install at the system level:

1. Build the release-varient shared object.
    - `make release`
1. Copy the shared object to a library directory.
    - `cp build/release/libkoi.so ~/local/lib/libkoi.so`
1. Ensure the library directory is in your `$LD_LIBRARY_PATH`
    - `echo "export LD_LIBRARY_PATH=~/.local/lib:\$LD_LIBRARY_PATH" >> ~/.bashrc`
    - `source ~/.bashrc`


Both of these approachs should allow you to run executables that depend on `libkoi.so`.





## Developing with Koi

### Prerequisites

To develop with Koi, you must be able to build Koi.

Regardless of if a system or local installation is performed, all compiling must:

- Use C++17 (`-std=c++17`)
- Link the koi, triton, and elf libraries (`-lkoi -ltriton -lelf`)

See the [examples](./examples) directory for example Makefiles that assume local installation.


### System Installation for Development

This is the path of least resistance, but requires you trust the security of this codebase:

1. Execute `sudo make dev`
    - Compiles a release-varient of the shared object
    - Copies the shared object to `/usr/local/lib`.
    - Copies the header files to `/usr/local/include`.


### Local Installation for Development

If you cannot (or do not want to) install at the system level:

1. Follow the steps to build Koi.
1. When compiling, include additional flags to:
    - Define the *directory* of `Swimmer.h` (`-I/path/to/Swimmer.h/directory/`)
    - Define the *directory* of `libkoi.so` (`-L/path/to/libkoi.so/directory/`)


### Documentation

In addition to the [Triton C++ API](https://triton-library.github.io/documentation/doxygen/annotated.html), the Koi features can be found in the [doc](./doc) directory.





## Examples

Multiple examples that solve real **CTF** challenges and detect varients of **buffer overflows** are available in the [examples](./examples) directory. However, a non-exhaustive way to **symbolize user input** may look like something this:

```cpp
#include <iostream>
#include <Koi/bait.h>
#include <Koi/swimmer.h>

triton::uint64 my_fgets(Swimmer *s, triton::uint64 addr) {
    // Do processing here
    std::cout << "TODO" << std::endl;

    /* Hook with bait to mark each input byte as symbolic
     * Assume fgets is called at 0x101200 to read n bytes
     * Symbolic variables are made to represent the buffer:
     * fgets<--0x101200[0x0]
     * fgets<--0x101200[0x1]
     * ...
     * fgets<--0x101200[0x(n-1)]  */
    return koi_fgets(s, addr);
}

int main(int argc, char *argv[]) {
    Swimmer swimmer = Swimmer("/path/to/my/elfFile");
    swimmer.verbosity = Swimmer::SV_ALL;

    const size_t MAIN_ENTRY_POINT = 0x1011eb;
    swimmer.setPc(MAIN_ENTRY_POINT);

    const size_t FGETS_ADDRESS = 0x1010d0;
    swimmer.hookFunction(FGETS_ADDRESS, my_fgets);

    const uint TARGET_ADDRESS = 0x1012dd;
    bool success = swimmer.explore(TARGET_ADDRESS);

    if(success) {
        auto model = swimmer.getSatModel();
        for(auto &pair : model) {
            std::cout << pair.second << std::endl;
        }
    }
}

```
