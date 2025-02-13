#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <Koi/swimmer.h>
#include <Koi/bait.h>
#include <Koi/stackframe.h>


/** On strcpy, validate enough memory is available at the destination */
triton::uint64 strcpy(Swimmer *s, triton::uint64 addr) {
    // Get registers for processing
    auto rdi = s->registers.x86_rdi;
    auto rsi = s->registers.x86_rsi;

    // Get the pointers in each register
    triton::uint64 dptr = triton::uint64(s->getConcreteRegisterValue(rdi));
    triton::uint64 sptr = triton::uint64(s->getConcreteRegisterValue(rsi));

    // Get the length of the string to be copied
    s->setConcreteRegisterValue(rdi, sptr);
    size_t slen = koi_strlen(s, addr) + 1;
    s->setConcreteRegisterValue(rdi, dptr);

    // Try all string length options for destination
    size_t dlen = s->readString(dptr).length() + 1;
    if(dlen == 1) {
        dlen = s->getAllocatedLength(dptr);
        if(dlen == 0) {
            dlen = s->getStackBufferLength(dptr);
        }
    }

    // Overflow detected
    if(dlen <= slen) {
        std::cout << "\033[1m\033[33mWARNING\033[0m\033[33m" << std::endl;

        // Check if null clobber or complete overflow
        if(dlen < slen) {
       	  std::cout << "Buffer overflow from strcpy @ 0x" << std::hex << addr << "..." << std::endl
                    << "Writing " << std::dec << slen << " characters to a buffer with length " << dlen << std::hex << std::endl;
        }

        // Heap-based buffer overflow
        if(s->isHeapAddress(dptr)) {
            std::cout << std::hex << "Victim buffer identified as " << s->getBufferAlias(dptr) << std::endl
                      << "  Allocated @ 0x" << s->getHeapOrigin(dptr) << std::endl;
        }

        // Stack-based buffer overflow
        else if(s->isStackAddress(dptr)) {
            std::cout << "Victim buffer is in the stack" << std::endl
                      << "  Local variables might be indirectly modified" << std::endl;

            // Stack smashing
            Stackframe *sf = s->getStackframe(dptr);
            triton::uint64 sfa = sf->getAddress();
            triton::uint64 sfs = sf->getSize();
            if(dptr + slen > sfa) {
                std::cout << "Stack smashing is imminent" << std::endl
                          << "  Victim buffer @ 0x" << dptr << std::endl
                          << "  Owning stackframe @ 0x" << sfa << std::endl
                          << "  0x" << dptr << " + 0x" << slen << " > " << "0x" << sfa << std::endl;
            }
        }
    }
    std::cout << std::dec << "\033[0m";

    // Perform the copy
    return koi_strcpy(s, addr);
}


/** Test case for bad_guard.c */
void bad_guard() {
    // Print a banner
    std::cout << ".-----------." << std::endl;
    std::cout << "| Bad Guard |" << std::endl;
    std::cout << "'-----------'" << std::endl;

    // Create the symbolic swimmer
    Swimmer swimmer = Swimmer("./input_exe/bad_guard");
    swimmer.setPc(0x101272);
    swimmer.verbosity = Swimmer::SV_CTRLFLOW | Swimmer::SV_MEM;

    // Hook the memory-related functions
    swimmer.hookFunction(0x101120, koi_fgets);
    swimmer.hookFunction(0x101130, koi_malloc);
    swimmer.hookFunction(0x1010c0, strcpy);
    swimmer.hookFunction(0x1010e0, koi_strlen);

    // Explore until after the overflow
    const uint TARGET_ADDRESS = 0x10126c;
    bool success = swimmer.explore(TARGET_ADDRESS);
    return;
}

/** Test case for function_stack.c */
void function_stack() {
    // Print a banner
    std::cout << ".-------------------------." << std::endl;
    std::cout << "| Symbolic Stack Overflow |" << std::endl;
    std::cout << "'-------------------------'" << std::endl;

    // Create the symbolic swimmer
    Swimmer swimmer = Swimmer("./input_exe/function_stack");
    swimmer.setPc(0x101232);
    swimmer.verbosity = Swimmer::SV_CTRLFLOW | Swimmer::SV_STACK;

    // Hook the memory-related functions
    swimmer.hookFunction(0x1010a0, strcpy);

    // Explore until after the overflow
    const uint TARGET_ADDRESS = 0x10121b;
    bool success = swimmer.explore(TARGET_ADDRESS);
    return;
}


/** Test case for simple_stack.c */
void simple_stack() {
    // Print a banner
    std::cout << ".-------------------------." << std::endl;
    std::cout << "| Concrete Stack Smashing |" << std::endl;
    std::cout << "'-------------------------'" << std::endl;

    // Create the symbolic swimmer
    Swimmer swimmer = Swimmer("./input_exe/simple_stack");
    swimmer.setPc(0x101189);
    swimmer.verbosity = Swimmer::SV_CTRLFLOW | Swimmer::SV_STACK;

    // Hook the memory-related functions
    swimmer.hookFunction(0x101070, strcpy);

    // Explore until after the overflow
    const uint TARGET_ADDRESS = 0x1011c2;
    bool success = swimmer.explore(TARGET_ADDRESS);
    return;
}


/** Test case for function_heap.c */
void function_heap() {
    // Print a banner
    std::cout << ".------------------------." << std::endl;
    std::cout << "| Symbolic Heap Overflow |" << std::endl;
    std::cout << "'------------------------'" << std::endl;

    // Create the symbolic swimmer
    Swimmer swimmer = Swimmer("./input_exe/function_heap");
    swimmer.setPc(0x10121d);
    swimmer.verbosity = Swimmer::SV_CTRLFLOW | Swimmer::SV_ALLOC;

    // Hook memory-related functions
    swimmer.hookFunction(0x1010f0, koi_malloc);
    swimmer.hookFunction(0x1010a0, strcpy);

    // Explore until after the overflow
    const uint TARGET_ADDRESS = 0x10121a;
    bool success = swimmer.explore(TARGET_ADDRESS);
    return;
}


/** Test case for simple_heap.c */
void simple_heap() {
    // Print a banner
    std::cout << ".------------------------." << std::endl;
    std::cout << "| Concrete Heap Overflow |" << std::endl;
    std::cout << "'------------------------'" << std::endl;

    // Create the symbolic swimmer
    Swimmer swimmer = Swimmer("./input_exe/simple_heap");
    swimmer.setPc(0x1011a9);
    swimmer.verbosity = Swimmer::SV_CTRLFLOW | Swimmer::SV_ALLOC;

    // Hook the memory-related functions
    swimmer.hookFunction(0x1010b0, koi_malloc);
    swimmer.hookFunction(0x101090, strcpy);

    // Explore until after the overflow
    const uint TARGET_ADDRESS = 0x1011e1;
    bool success = swimmer.explore(TARGET_ADDRESS);
    return;
}


/** Example to showcase the detection of several buffer overflow varients */
int main(int argc, char *argv[]) {
    // Overflow from strcpy of concrete heap string to a small heap buffer
    simple_heap();
    std::cout << std::endl;

    // // Overflow from strcpy of symbolic heap string to a small heap buffer
    function_heap();
    std::cout << std::endl;

    // Overflow from strcpy of concrete stack string to small stack buffer
    simple_stack();
    std::cout << std::endl;

    // Overflow from strcpy of symbolic stack string to small stack buffer
    function_stack();
    std::cout << std::endl;

    // Overflow from strcpy of concrete stack string to small heap buffer
    // Overflow occurs due to a bad guard that many novices make
    bad_guard();
    std::cout << std::endl;

    return 0;
}
