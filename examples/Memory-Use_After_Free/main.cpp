#include <iomanip>
#include <iostream>
#include <Koi/swimmer.h>
#include <Koi/bait.h>

/**
 * On fprintf, check for use after free
 * This could be extendend to calculate how many memory pointers to check
 * instead of only checking the first one. However, this is sufficient
 * for our simple use case
 */
triton::uint64 checkUseAfterFree(Swimmer *s, triton::uint64 addr) {
    auto rdx = s->registers.x86_rdx;
    triton::uint64 ptr = triton::uint64(s->getConcreteRegisterValue(rdx));
    if(!s->statHeapMemory(ptr, true)) {
        std::cout << "\033[1m\033[33mWARNING\033[0m\033[33m" << std::endl
                  << "Use-after-free @ 0x" << std::hex << addr << "..." << std::endl
                  << "Offending buffer identified as " << s->getBufferAlias(ptr) << std::endl
                  << "  Allocated @ 0x" << s->getHeapOrigin(ptr) << std::endl
                  << "  Free'd @ 0x" << s->getHeapSink(ptr)
                  << "\033[0m" << std::dec << std::endl;
    }
    return 0;
}


/** Example to showcase the detection of several buffer overflow varients */
int main(int argc, char *argv[]) {
    // Print a banner
    std::cout << ".----------------." << std::endl;
    std::cout << "| Use After Free |" << std::endl;
    std::cout << "'----------------'" << std::endl;

    // Create the symbolic swimmer
    Swimmer swimmer = Swimmer("./input_exe/use_after_free");
    swimmer.setPc(0x10170b);
    swimmer.verbosity = Swimmer::SV_CTRLFLOW | Swimmer::SV_MEM;

    // Hook the memory-related functions
    swimmer.hookFunction(0x101190, koi_malloc);
    swimmer.hookFunction(0x101100, koi_free);
    swimmer.hookFunction(0x101180, checkUseAfterFree);

    // Explore until after the use after free
    bool success = swimmer.explore(0x10158b, 1, 3);

    // If target not reached
    if(!success) {
        std::cout << "unsat" << std::endl;
        return 1;
    }
    std::cout << std::endl;

    // Get the satisfying model
    auto model = swimmer.getSatModel();

    // Sort the keys
    std::vector<int> keys;
    for(const auto& pair : model) {
        keys.push_back(pair.first);
    }
    std::sort(keys.begin(), keys.end());

    for(const int& key : keys) {
        auto assignment = model[key];
        std::cout << key << ":\t" << assignment << std::endl;
    }

    return 0;
}