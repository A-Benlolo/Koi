#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <Koi/bait.h>
#include <Koi/swimmer.h>


/** On scanf, symbolize the amount of memory that input would take */
triton::uint64 __isoc99_scanf(Swimmer *s, triton::uint64 addr) {
    // Get registers for processing
    auto rdi = s->registers.x86_rdi;
    auto rsi = s->registers.x86_rsi;

    // Memory address where first input is stored
    triton::uint64 data_ptr = triton::uint64(s->getConcreteRegisterValue(rsi));

    // Type of input (non-exhaustive)
    triton::uint64 fmt_ptr = triton::uint64(s->getConcreteRegisterValue(rdi));
    std::string fmt = s->readString(fmt_ptr);
    uint len = 0;
    if(fmt == "%d" || fmt == "%x")
        len = 4;

    // Symbolize memory
    s->symbolizeNamedMemoryChunk("__isoc99_scanf", data_ptr, addr, len);
    return 4;
}


/** On strcmp, assert equality at the next jump */
triton::uint64 strcmp(Swimmer *s, triton::uint64 addr) {
    // Get registers for processing
    auto rdi = s->registers.x86_rdi;
    auto rsi = s->registers.x86_rsi;
    auto rax = s->registers.x86_rax;

    // Memory address where strings are stored
    triton::uint64 str1_ptr = triton::uint64(s->getConcreteRegisterValue(rdi)); // points to symbolic
    triton::uint64 str2_ptr = triton::uint64(s->getConcreteRegisterValue(rsi)); // points to concrete

    // Read the concrete string
    std::string str2 = s->readString(str2_ptr);

    // Create the constraint of equality between the two memory regions
    triton::ast::SharedAstContext astCtxt = s->getAstContext();
    std::vector<triton::ast::SharedAbstractNode> equality;
    for(size_t i = 0; i < str2.size(); i++) {
        auto symMem = s->getSymbolicMemory(str1_ptr + i)->getAst();
        auto expected = astCtxt->equal(symMem, astCtxt->bv(str2[i], 8));
        equality.push_back(expected);
    }

    // Inject the new jump condition
    s->injectJumpCondition(0x101288, astCtxt->lnot(astCtxt->land(equality)));
    return 0;
}


/** On putchar, update a global flag */
static std::string flag = "";
triton::uint64 putchar(Swimmer *s, triton::uint64 addr) {
    flag += triton::uint8(s->getConcreteRegisterValue(s->registers.x86_rdi));
    return 0;
}


/** Example to showcase crackmes.one sample (strcmp) */
int main(int argc, char *argv[]) {
    // Print a banner
    std::cout << ".-------------------------." << std::endl;
    std::cout << "| Simple strcmp Injection |" << std::endl;
    std::cout << "'-------------------------'" << std::endl;

    // Create the symbolic swimmer
    Swimmer swimmer = Swimmer("./6784f8a84d850ac5f7dc5173");
    swimmer.setPc(0x1011e9);
    swimmer.verbosity = Swimmer::SV_CTRLFLOW;

    // Hook library functions
    swimmer.hookFunction(0x1010d0, koi_fgets);
    swimmer.hookFunction(0x1010f0, __isoc99_scanf);
    swimmer.hookFunction(0x1010e0, strcmp); // Implements a jump injection
    swimmer.hookFunction(0x1010a0, putchar);

    // Explore to the address where the flag is printed
    const uint TARGET_ADDRESS = 0x1012dd;
    const uint MAX_VISITS = 0x20;
    const uint MAX_DEPTH = 4;
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

    // Build the password and key from the sorted keys
    std::string password = "";
    size_t secret_code = 0;
    for(const int& key : keys) {
        auto assignment = model[key];
        std::cout << key << ": "<< assignment;
        if(assignment.getValue() <= 0xFF) {
            triton::uint8 byte = triton::uint8(assignment.getValue());
            if(byte == 0x0a)
                std::cout << " \\n";
            else {
                std::cout << " " << byte;
                password += byte;
            }
        }
        else {
            secret_code = triton::uint64(assignment.getValue());
        }
        std::cout << std::endl;
    }
    std::cout << std::endl << "\033[1mPassword: \033[0m" << password << std::endl;
    std::cout << "\033[1mSecret Code: \033[0m" << secret_code << std::endl;
    std::cout << "\033[1mRecovered Flag: \033[0m" << flag << std::endl << std::endl;
    return 0;
}