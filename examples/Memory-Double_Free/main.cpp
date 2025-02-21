#include <iomanip>
#include <iostream>
#include <Koi/swimmer.h>
#include <Koi/bait.h>

/**
 * On free, check for a double free
 */
triton::uint64 checkDoubleFree(Swimmer *s, triton::uint64 addr) {
    auto rdi = s->registers.x86_rdi;
    triton::uint64 ptr = triton::uint64(s->getConcreteRegisterValue(rdi));
    if(!s->statHeapMemory(ptr, true)) {
        std::cout << "\033[1m\033[33mWARNING\033[0m\033[33m" << std::endl
                  << "Double free @ 0x" << std::hex << addr << "..." << std::endl
                  << "Offending buffer identified as " << s->getBufferAlias(ptr) << std::endl
                  << "  Allocated @ 0x" << s->getHeapOrigin(ptr) << std::endl
                  << "  Free'd @ 0x" << s->getHeapSink(ptr)
                  << "\033[0m" << std::dec << std::endl;
    }
    return koi_free(s, addr);
}


/** Example to showcase the detection of several buffer overflow varients */
int main(int argc, char *argv[]) {
    // Print a banner
    std::cout << ".-------------." << std::endl;
    std::cout << "| Double Free |" << std::endl;
    std::cout << "'-------------'" << std::endl;

    // Create the symbolic swimmer
    Swimmer swimmer = Swimmer("./input_exe/double_free");
    swimmer.setPc(0x101169);
    swimmer.verbosity = Swimmer::SV_CTRLFLOW | Swimmer::SV_MEM;

    // Hook the memory-related functions
    swimmer.hookFunction(0x101070, koi_malloc);
    swimmer.hookFunction(0x101060, checkDoubleFree);

    // Explore everything
    swimmer.explore();
    return 0;
}