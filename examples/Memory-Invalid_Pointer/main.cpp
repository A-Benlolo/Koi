#include <iomanip>
#include <iostream>
#include <Koi/swimmer.h>
#include <Koi/bait.h>

/**
 * On free, check that the pointer is valid
 */
triton::uint64 checkInvalidPointer(Swimmer *s, triton::uint64 addr) {
    auto rdi = s->registers.x86_rdi;
    triton::uint64 ptr = triton::uint64(s->getConcreteRegisterValue(rdi));
    if(!s->isHeapStub(ptr)) {
        triton::uint64 owner = s->getHeapStub(ptr);
        std::cout << "\033[1m\033[33mWARNING\033[0m\033[33m" << std::endl
                  << "Invalid pointer @ 0x" << std::hex << addr << "..." << std::endl
                  << "Address is contained by buffer " << s->getBufferAlias(owner) << std::endl
                  << "  Attempted to free 0x" << ptr << std::endl
                  << "  Must instead free 0x" << owner << std::endl
                  << "\033[0m" << std::dec << std::endl;
    }
    return koi_free(s, addr);
}


/** Example to showcase the detection of several buffer overflow varients */
int main(int argc, char *argv[]) {
    // Print a banner
    std::cout << ".-----------------." << std::endl;
    std::cout << "| Invalid Pointer |" << std::endl;
    std::cout << "'-----------------'" << std::endl;

    // Create the symbolic swimmer
    Swimmer swimmer = Swimmer("./input_exe/invalid_pointer");
    swimmer.setPc(0x1011c9);
    swimmer.verbosity = Swimmer::SV_CTRLFLOW | Swimmer::SV_MEM;

    // Hook the memory-related functions
    swimmer.hookFunction(0x1010c0, koi_calloc);
    swimmer.hookFunction(0x101090, checkInvalidPointer);
    swimmer.hookFunction(0x1010a0, koi_strchr);

    // Explore everything
    swimmer.explore();

    return 0;
}