#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <Koi/bait.h>
#include <Koi/swimmer.h>


/** Print information on the parameters passed to generate_password */
void checkParams(Swimmer *s, triton::arch::Instruction insn) {
    auto rdi = s->registers.x86_rdi;
    auto rsi = s->registers.x86_rsi;

    triton::uint64 sexy1337_ptr = triton::uint64(s->getConcreteRegisterValue(rsi));
    triton::uint64 key_ptr = triton::uint64(s->getConcreteRegisterValue(rdi));
    bool keyIsSymbolized = s->isMemorySymbolized(key_ptr);

    std::cout << "\t\"sexy1337\" @ 0x" << std::hex << sexy1337_ptr << std::dec << std::endl;
    std::cout << "\tkey_ptr @ 0x" << std::hex << key_ptr << std::dec << std::endl;
    std::cout << "\tkey_ptr has been symbolized: " << keyIsSymbolized << std::endl;
}


/** Print the result of generate_password */
void checkReturn(Swimmer *s, triton::arch::Instruction insn) {
    auto rax = s->registers.x86_rax;
    triton::uint64 key_ptr = triton::uint64(s->getConcreteRegisterValue(rax));

    std::cout << "\tkey_ptr @ 0x" << std::hex << key_ptr << std::dec << std::endl;
    std::cout << "\t\033[1mgenerate_password \033[0m--> " << s->readString(key_ptr) << std::endl;
}


/** Example to showcase crackmes.one sample (simple XOR cipher) */
int main(int argc, char *argv[]) {
    // Print a banner
    std::cout << ".-------------------." << std::endl;
    std::cout << "| Simple XOR Cipher |" << std::endl;
    std::cout << ".-------------------." << std::endl;

    // Create the symbolic swimmer
    Swimmer swimmer = Swimmer("./6715466c9b533b4c22bd18bb");
    swimmer.setPc(0x101209);
    swimmer.verbosity = Swimmer::SV_CTRLFLOW;

    // Hook the strlen function
    swimmer.hookFunction(0x1010c0, koi_strlen);

    // Hooks to demonstrate solution
    swimmer.hookInstruction(0x10134d, checkParams);
    swimmer.hookInstruction(0x10123a, checkReturn);

    // Explore until the expected password is made
    const uint TARGET_ADDRESS = 0x10123a;
    const uint MAX_VISITS = 0;
    const uint MAX_DEPTH = 0;
    bool success = swimmer.explore(TARGET_ADDRESS, MAX_VISITS, MAX_DEPTH);
    return 0;
}