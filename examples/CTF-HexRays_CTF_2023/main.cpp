#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <Koi/swimmer.h>

/** On strncpy, symbolize the dst with n bytes */
triton::uint64 strncpy(Swimmer *s, triton::uint64 addr) {
    // Get registers that store function call parameters
    auto rdi = s->registers.x86_rdi;
    auto rsi = s->registers.x86_rsi;
    auto rdx = s->registers.x86_rdx;

    // Length is concrete
    triton::uint64 n = triton::uint64(s->getConcreteRegisterValue(rdx));

    // Src is symbolic (argv[1]), so symbolize dst with n bytes and a known null byte
    triton::uint64 dstAddr = triton::uint64(s->getConcreteRegisterValue(rdi));
    triton::ast::SharedAstContext astCtxt = s->getAstContext();
    for(triton::uint64 i = 0; i <= n; i++) {
        std::stringstream ss;
        ss << "strncpy<--0x" << std::hex << addr << "[0x" << std::setw(2) << std::setfill('0') << i << "]";
        s->symbolizeMemory(triton::arch::MemoryAccess(dstAddr + i, 1), ss.str());

        auto symMem = s->getSymbolicMemory(dstAddr + i)->getAst();
        if(i == n) {
            s->cnstrs.push_back(astCtxt->equal(symMem, astCtxt->bv(0x00, 8)));
        }
        else {
            auto asciiLo = astCtxt->bvuge(symMem, astCtxt->bv(0x20, 8));
            auto asciiHi = astCtxt->bvule(symMem, astCtxt->bv(0x7E, 8));
            s->cnstrs.push_back(astCtxt->land(asciiLo, asciiHi));
        }
    }

    // A copy of destination is returned
    return dstAddr;
}


/** Break out of decrypt loop 1 */
void breakDecryptLoop_1(Swimmer *s, triton::arch::Instruction insn) {
    std::cout << "\033[1mForcing break from loop 1\033[0m" << std::endl;
    s->setConcreteRegisterValue(s->registers.x86_rcx, 0x1D9AD);
    s->processing(insn);
    return;
}


/** Break out of decrypt loop 2 */
void breakDecryptLoop_2(Swimmer *s, triton::arch::Instruction insn) {
    std::cout << "\033[1mForcing break from loop 2\033[0m" << std::endl;
    s->setConcreteRegisterValue(s->registers.x86_rdi, 0x3B35A + 1);
    s->processing(insn);
    return;
}


/** Example to showcase HexRays 2023 CTF Challenge (SMT necessary) */
int main(int argc, char *argv[]) {
    // Print a banner
    std::cout << ".----------------------------." << std::endl;
    std::cout << "| HexRays 2023 CTF Challenge |" << std::endl;
    std::cout << "'----------------------------'" << std::endl;

    // Create the symbolic swimmer
    Swimmer swimmer = Swimmer("./0f89421f3c52c90b0789b90f");
    swimmer.setPc(0x101220);
    swimmer.verbosity = Swimmer::SV_CTRLFLOW;

    // Hook strncpy to symbolize the dst when the src is symbolic
    swimmer.hookFunction(0x101180, strncpy);

    // Break out of the decryption loops after the first iteration
    swimmer.hookInstruction(0x101414, breakDecryptLoop_1);
    swimmer.hookInstruction(0x10147A, breakDecryptLoop_2);

    // Explore to the address where the good image is loaded
    const uint TARGET_ADDRESS = 0x10149b;
    const uint MAX_VISITS = 1;
    const uint MAX_DEPTH = 0;
    bool success = swimmer.explore(TARGET_ADDRESS, MAX_VISITS, MAX_DEPTH);

    // On failure, finish
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

    // Build the answer from the sorted keys
    std::string answer = "";
    for(const int& key : keys) {
        auto assignment = model[key];
        std::cout << key << ": "<< assignment;
        if(key > 10) {
            triton::uint8 byte = triton::uint8(assignment.getValue());
            std::cout << " " << byte;
            if(byte == 0x00) break;
            answer += byte;
        }
        std::cout << std::endl;
    }
    std::cout << std::endl << std::endl << "\033[1mAnswer: \033[0m" << answer << std::endl << std::endl;
    return 0;
}