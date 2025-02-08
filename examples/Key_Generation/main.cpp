#include <iomanip>
#include <iostream>
#include <string>
#include <vector>
#include <Swimmer.h>


/** Restrict input to be the minimal viable length (including a null byte)
    I've successfully generated keys of length 128, but it took 1m20s. */
const static size_t INPUT_LENGTH = 6;


/** Spoof the length of argv[1] in check_id_xor and check_id_sum */
void spoofLength(Swimmer *s, triton::arch::Instruction insn) {
    s->setConcreteRegisterValue(s->registers.x86_rax, INPUT_LENGTH - 1);
}


/** Implement guard to have a minimum argv[1] length of 5 */
void constraint1(Swimmer *s, triton::arch::Instruction insn) {
    // Get the pointer to the data and AST context
    triton::uint64 ptr = triton::uint64(s->getConcreteRegisterValue(s->registers.x86_rdi));
    triton::ast::SharedAstContext ac = s->getAstContext();

    // Symbolize newly known memory
    for(size_t i = 0; i < 6; i++) {
        std::stringstream ss;
        ss << "argv[0x" << std::hex << i << "]" << std::dec;
        triton::arch::MemoryAccess mem = triton::arch::MemoryAccess(ptr + i, 1);
        s->symbolizeMemory(mem, ss.str());
    }

    // Constrain memory
    std::vector<triton::ast::SharedAbstractNode> cnstrs;
    for(size_t i = 0; i < 5; i++) {
        auto memAst = s->getSymbolicMemory(ptr + i)->getAst();
        cnstrs.push_back(ac->equal(memAst, ac->bv(0x00, 8)));
    }

    // Implement new jump condition
    // We want to negate this condition, so OR is used
    s->injectJumpCondition(0x101360, ac->lor(cnstrs));

    // Return the spoofed length
    return spoofLength(s, insn);
}


/** Implement guard to have a real argv[1] length of INPUT_LENGTH */
void constraint2(Swimmer *s, triton::arch::Instruction insn) {
    // Get the pointer to the data and AST context
    triton::uint64 ptr = triton::uint64(s->getConcreteRegisterValue(s->registers.x86_rdi));
    triton::ast::SharedAstContext ac = s->getAstContext();

    // Symbolize newly known memory
    for(size_t i = 6; i < INPUT_LENGTH; i++) {
        std::stringstream ss;
        ss << "argv[0x" << std::hex << i << "]" << std::dec;
        triton::arch::MemoryAccess mem = triton::arch::MemoryAccess(ptr + i, 1);
        s->symbolizeMemory(mem, ss.str());
    }

    // Constrain the newly known memory
    std::vector<triton::ast::SharedAbstractNode> cnstrs;
    for(size_t i = 6; i < INPUT_LENGTH; i++) {
        auto memAst = s->getSymbolicMemory(ptr + i)->getAst();
        cnstrs.push_back(ac->equal(memAst, ac->bv(0x00, 8)));
    }

    // Implement new jump condition
    // NOTE: Only one byte is allowed to be null, so XOR is used
    if(cnstrs.size() > 1)
        s->injectJumpCondition(0x10137b, ac->lxor(cnstrs));
    else if(cnstrs.size() == 1)
        s->injectJumpCondition(0x10137b, cnstrs[0]);

    // Return the spoofed length
    return spoofLength(s, insn);
}


/** Implement guard to have all non-null characters in argv[1] be numeric-ASCII */
void constraint3(Swimmer *s, triton::arch::Instruction insn) {
    // Get the pointer to the data and AST context
    triton::uint64 ptr = triton::uint64(s->getConcreteRegisterValue(s->registers.x86_rdi));
    triton::ast::SharedAstContext ac = s->getAstContext();

    // Constrain memory
    std::vector<triton::ast::SharedAbstractNode> cnstrs;
    for(size_t i = 0; i < INPUT_LENGTH - 1; i++) {
        auto memAst = s->getSymbolicMemory(ptr + i)->getAst();
        auto numLo = ac->bvuge(memAst, ac->bv(0x30, 8));
        auto numHi = ac->bvule(memAst, ac->bv(0x39, 8));
        cnstrs.push_back(ac->land(numLo, numHi));
    }

    // Implement new jump condition
    s->injectJumpCondition(0x1013ce, ac->land(cnstrs));

    // Ensure RAX and RBX are equal
    s->setConcreteRegisterValue(s->registers.x86_rax, 0);
    s->setConcreteRegisterValue(s->registers.x86_rbx, 0);
    return;
}


/** On strycpy, constrain global PASS to be equal to argv[1] */
void strcpy(Swimmer *s, triton::uint64 addr) {
    // Get concrete addresses of strings
    triton::uint64 dst_ptr = triton::uint64(s->getConcreteRegisterValue(s->registers.x86_rdi));
    triton::uint64 src_ptr = triton::uint64(s->getConcreteRegisterValue(s->registers.x86_rsi));

    // Symbolize the destinination bytes
    for(size_t i = 0; i < INPUT_LENGTH; i++) {
        std::stringstream ss;
        ss << "PASS[0x" << std::hex << i << "]" << std::dec;
        triton::arch::MemoryAccess mem = triton::arch::MemoryAccess(dst_ptr + i, 1);
        s->symbolizeMemory(mem, ss.str());
    }

    // Constraint the destinination to be equal to the source
    triton::ast::SharedAstContext ac = s->getAstContext();
    for(size_t i = 0; i < INPUT_LENGTH; i++) {
        auto srcByteAst = s->getSymbolicMemory(src_ptr + i)->getAst();
        auto dstByteAst = s->getSymbolicMemory(dst_ptr + i)->getAst();
        s->cnstrs.push_back(ac->equal(srcByteAst, dstByteAst));
    }
}


/** Example to showcase crackmes.one sample (key generation) */
int main(int argc, char *argv[]) {
    // Print a banner
    std::cout << ".----------------." << std::endl;
    std::cout << "| Key Generation |" << std::endl;
    std::cout << "'----------------'" << std::endl;

    // Create the symbolic swimmer
    Swimmer swimmer = Swimmer("./6784563e4d850ac5f7dc5137");
    swimmer.setPc(0x10130a);
    swimmer.verbosity = Swimmer::SV_CTRLFLOW;

    // Seed program parameters addresses to store *argv[1] at 0xDEADBEFF instead of 0x0
    swimmer.setConcreteRegisterValue(swimmer.registers.x86_rsi, 0xBA5EBA11);
    swimmer.setConcreteMemoryAreaValue(0xBA5EBA11 + 8, { 0xEF, 0xBE, 0xAD, 0xDE });

    // Hook checks that rely on library functions
    swimmer.hookInstruction(0x101357, constraint1);
    swimmer.hookInstruction(0x101370, constraint2);
    swimmer.hookInstruction(0x1013A9, constraint3);
    swimmer.hookFunction(0x1010a0, strcpy);
    swimmer.hookInstruction(0x10122a, spoofLength);
    swimmer.hookInstruction(0x1012db, spoofLength);

    // Kill branches that lead to down bad paths
    swimmer.killAddress(0x1010f0); // Exit function
    swimmer.killAddress(0x1011e9); // Error function
    swimmer.killAddress(0x101260); // Fail check_id_xor
    swimmer.killAddress(0x1012f7); // Fail check_id_sum

    // Explore to the address where the success message is printed
    const uint TARGET_ADDRESS = 0x10141a;
    const uint MAX_VISITS = 0;
    const uint MAX_DEPTH = 8;
    bool success = swimmer.explore(TARGET_ADDRESS, MAX_VISITS, MAX_DEPTH);

    // On failure, finish
    if(!success) {
        std::cout << "unsat" << std::endl;
        return 1;
    }
    std::cout << std::endl;

    // Get satisfying models
    auto models = swimmer.getSatModels(3);

    // Iterate all models to display mulitple keys
    for(size_t i = 0; i < models.size(); i++) {
        // Sort the ModelSolver keys
        std::vector<int> keys;
        for(const auto& pair : models[i]) {
            keys.push_back(pair.first);
        }
        std::sort(keys.begin(), keys.end());

        // Build a valid key from the sorted ModelSolver keys
        std::string validKey = "";
        for(const int& key : keys) {
            auto assignment = models[i][key];
            if(key >= 30) {
                triton::uint8 byte = triton::uint8(assignment.getValue());
                if(byte == 0x00) break;
                validKey += byte;
            }
        }
        std::cout << "\033[1mKey " << (i + 1) << ": \033[0m" << validKey << std::endl;
    }
    return 0;
}