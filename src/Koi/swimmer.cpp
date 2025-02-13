#include <fstream>
#include <iomanip>
#include <iostream>
#include <optional>
#include <sstream>
#include <vector>
#include <triton/context.hpp>
#include <triton/x86Specifications.hpp>
#include <triton/ast.hpp>
#include "Koi/buffer.h"
#include "Koi/stackframe.h"
#include "Koi/swimmer.h"
#include "elfivator.h"


/********************/
/* HELPER FUNCTIONS */
/********************/


/**
 * Get the children of an ite AbstractNode from an instruction if possible.
 * @param insn - Instruction to parse
 * @return a vector of AbstractNodes that represent if-then-else.
 */
std::vector<triton::ast::SharedAbstractNode> getIte(triton::arch::Instruction insn) {
    std::vector<triton::ast::SharedAbstractNode> ite;
    for(const auto& expr: insn.symbolicExpressions) {
        const triton::ast::SharedAbstractNode& ast = expr->getAst();
        if(ast->getType() != triton::ast::ITE_NODE) continue;
        ite = ast->getChildren();
        if(ite[0]->isSymbolized() && !ite[1]->isSymbolized() && !ite[2]->isSymbolized())
            break;
        ite.clear();
    }
    return ite;
}


/********************/
/* PUBLIC FUNCTIONS */
/********************/


/**
 * Constructor
 * @param filein - Path to a file to load.
 * @return a new Swimmer
 */
Swimmer::Swimmer(const std::string& filein) : triton::Context(triton::arch::ARCH_X86_64) {
    // Registers with assumed starting values
    // TODO: This should be handled by the loader
    setConcreteRegisterValue(registers.x86_rip, 0);
    setConcreteRegisterValue(registers.x86_rbp, STACK_START);
    setConcreteRegisterValue(registers.x86_rsp, STACK_START);

    // General-use registers
    symbolizeRegister(registers.x86_rax, "symbolic_rax");
    symbolizeRegister(registers.x86_rbx, "symbolic_rbx");
    symbolizeRegister(registers.x86_rcx, "symbolic_rcx");
    symbolizeRegister(registers.x86_rdx, "symbolic_rdx");
    symbolizeRegister(registers.x86_rsi, "symbolic_rsi");
    symbolizeRegister(registers.x86_rdi, "symbolic_rdi");
    symbolizeRegister(registers.x86_r8, "symbolic_r8");
    symbolizeRegister(registers.x86_r9, "symbolic_r9");
    symbolizeRegister(registers.x86_r10, "symbolic_r10");
    symbolizeRegister(registers.x86_r11, "symbolic_r11");
    symbolizeRegister(registers.x86_r12, "symbolic_r12");
    symbolizeRegister(registers.x86_r13, "symbolic_r13");
    symbolizeRegister(registers.x86_r14, "symbolic_r14");
    symbolizeRegister(registers.x86_r15, "symbolic_r15");

    // Common flag registers
    symbolizeRegister(registers.x86_cf, "symbolic_cf");
    symbolizeRegister(registers.x86_of, "symbolic_of");
    symbolizeRegister(registers.x86_pf, "symbolic_pf");
    symbolizeRegister(registers.x86_sf, "symbolic_sf");
    symbolizeRegister(registers.x86_tf, "symbolic_tf");
    symbolizeRegister(registers.x86_zf, "symbolic_zf");

    // XMM registers
    symbolizeRegister(registers.x86_xmm0, "symbolic_xmm0");
    symbolizeRegister(registers.x86_xmm1, "symbolic_xmm1");
    symbolizeRegister(registers.x86_xmm2, "symbolic_xmm2");
    symbolizeRegister(registers.x86_xmm3, "symbolic_xmm3");
    symbolizeRegister(registers.x86_xmm4, "symbolic_xmm4");
    symbolizeRegister(registers.x86_xmm5, "symbolic_xmm5");
    symbolizeRegister(registers.x86_xmm6, "symbolic_xmm6");
    symbolizeRegister(registers.x86_xmm7, "symbolic_xmm7");
    symbolizeRegister(registers.x86_xmm8, "symbolic_xmm8");
    symbolizeRegister(registers.x86_xmm9, "symbolic_xmm9");
    symbolizeRegister(registers.x86_xmm10, "symbolic_xmm10");
    symbolizeRegister(registers.x86_xmm11, "symbolic_xmm11");
    symbolizeRegister(registers.x86_xmm12, "symbolic_xmm12");
    symbolizeRegister(registers.x86_xmm13, "symbolic_xmm13");
    symbolizeRegister(registers.x86_xmm14, "symbolic_xmm14");
    symbolizeRegister(registers.x86_xmm15, "symbolic_xmm15");

    // Initialize the stackframe as not existing
    stackframes.push_back(Stackframe(STACK_START, 0));

    // Load the bytes using Elfivator
    Elfivator e = Elfivator(filein);
    for(auto& section : e.sections) {
        if(section.name != ".plt.sec")
            setConcreteMemoryAreaValue(section.offset + 0x100000, section.data);
    }
}


/**
 * Set the instruction pointer (pc) value.
 * @param x - Value to set to.
 */
void Swimmer::setPc(triton::uint64 x) {
    setConcreteRegisterValue(registers.x86_rip, x);
}


/**
 * Explores the memory pool from the instruction pointer, respecting hooks and injections
 * @param target - Desired address to execute
 * @param maxVisits - Maximum number of times to execute the same instruction
 * @param maxDepth - Maximum fork depth of an execution branch
 * @return if the target was reached (default=False)
 */
bool Swimmer::explore(triton::uint64 target, uint maxVisits, uint maxDepth) {
    uint localFid = fid;
    depth++;
    fid++;

    // Iterate until one of many stopping conditions
    while(true) {
        // Get the instruction pointer
        triton::uint64 pc = triton::uint64(getConcreteRegisterValue(registers.x86_rip));

        // Ensure the instruction has not been visited too many times
        if(maxVisits > 0) {
            if(++visits[pc] > maxVisits) {
                if(verbosity & SV_STOPS)
                    std::cout << "\033[31mExhausted 0x" << std::setfill('0') << std::hex << pc << "\033[0m" << std::dec << std::endl;
                break;
            }
        }

        // Get the instruction bytes if they have been defined
        if(!isConcreteMemoryValueDefined(pc, 1)) {
            if(verbosity & SV_STOPS)
                std::cout << "\033[31mUndefined: 0x" << std::hex << pc << "\033[0m" << std::dec << std::endl;
            break;
        }
        std::vector<triton::uint8> opcode = getConcreteMemoryAreaValue(pc, 16);

        // Initialize the next instruction
        triton::arch::Instruction insn = injectedInstructions.count(pc)
                                       ? injectedInstructions[pc]
                                       : triton::arch::Instruction(pc, opcode.data(), 16);

        // Process the instruction
        processing(insn);
        triton::uint32 insnType = insn.getType();
        if(verbosity & SV_INSN)
            std::cout << "[" << localFid << "] (" << depth << ") " << insn << std::endl;
        if(verbosity & SV_REGS)
            __printRegisters();

        // Restore semantics of an injected instruction
        if(injectedInstructions.count(pc)) {
            insn.symbolicExpressions = injectedInstructions[pc].symbolicExpressions;
            disassembly(insn);
        }

        // Perform address/instruction hooks
        if(insnHooks.count(pc)) {
            for(InsnHook& callback : insnHooks[pc]) {
                callback(this, insn);
            }
        }

        // Return success if target is reached
        if(target != 0 & pc == target) {
            if(verbosity & SV_STOPS)
                std::cout << "\033[32mTarget Reached\033[0m" << std::endl;
            return true;
        }

        // Return failure if dead end is reached
        if(std::find(deadEnds.begin(), deadEnds.end(), pc) != deadEnds.end()) {
            if(verbosity & SV_STOPS)
                std::cout << "\033[31mDead End Reached\033[0m" << std::endl;
            return false;
        }

        // Handle stackframe information
        // This idoes not disqualify other handlers
        __handleStackAllocation(insn);
        __handelStackReference(insn);

        // Break on halt
        if(insnType == triton::arch::x86::ID_INS_HLT)
            break;

        // Break on return when the next instruction is fallthrough
        else if(insnType == triton::arch::x86::ID_INS_RET) {
            if(getConcreteRegisterValue(registers.x86_rip) == 0) {
                std::cout << "\033[31mEnd of Path Reached\033[0m" << std::endl;
                break;
            } else if (verbosity & SV_STACK) {
                stackframes.pop_back();
                std::cout << "\033[1mEnd of stackframe\033[0m" << std::endl;
            }
        }

        // Handle calls to unknown memory by skipping or hooking
        else if(__handleCall(localFid, pc, insn))
            continue;

        // Check for new symbolic stack variables
        else if(__handleMemoryRead(pc, insn))
            continue;

        // Fork at symbolic conditional branch
        else if(insn.isBranch() && insn.isSymbolized() && insnType != triton::arch::x86::ID_INS_JMP) {
            // Forking is only possible if an instruction is symbolized
            std::vector<triton::ast::SharedAbstractNode> ite = getIte(insn);

            // The symbolic statement was found
            if(ite.size() == 3) {
                triton::ast::SharedAstContext astCtxt = getAstContext();

                // Determine satisfiable model for "if"
                cnstrs.push_back(ite[0]);
                triton::ast::SharedAbstractNode cnstr_if = cnstrs.size() > 1 ? astCtxt->land(cnstrs) : cnstrs[0];
                std::unordered_map<long unsigned int, triton::engines::solver::SolverModel> model_if = getModel(cnstr_if);
                cnstrs.pop_back();

                // Determine satisfiable model for "else"
                cnstrs.push_back(astCtxt->lnot(ite[0]));
                triton::ast::SharedAbstractNode cnstr_else = cnstrs.size() > 1 ? astCtxt->land(cnstrs) : cnstrs[0];
                std::unordered_map<long unsigned int, triton::engines::solver::SolverModel> model_else = getModel(cnstr_else);
                cnstrs.pop_back();

                // Only fork if both satisfiable, else defer to Triton
                if(model_if.size() > 0 && model_else.size() > 0) {
                    // Verify exection depth is not too complex
                    if(maxDepth > 0 && depth >= maxDepth) {
                        if(verbosity & SV_STOPS)
                            std::cout << "\033[31mToo deep to fork\033[0m" << std::endl;
                        continue;
                    }

                    // Recursively follow the jump
                    else {
                        setConcreteRegisterValue(registers.x86_rip, ite[1]->evaluate());
                        cnstrs.push_back(cnstr_if);
                        if(verbosity & SV_BRANCH)
                            std::cout << "\033[1mJUMP from 0x" << std::hex << pc << std::dec << "\033[0m" << std::endl;
                        if(verbosity & SV_MODEL) {
                            for (const auto& pair : model_if) {
                                std::cout << "\t" << pair.first << ": "<< pair.second << std::endl;
                            }
                        }

                        // TODO: Save and restore all registers, not just RBP
                        // TODO: This will need to be done for memory as well
                        triton::uint64 rbpBefore = triton::uint64(getConcreteRegisterValue(registers.x86_rbp));
                        if(explore(target, maxVisits, maxDepth))
                            return true;
                        cnstrs.pop_back();
                        setConcreteRegisterValue(registers.x86_rbp, rbpBefore);

                        // Restore to fallthrough
                        if(verbosity & SV_BRANCH)
                            std::cout << "\033[1mFALL from 0x" << std::hex << pc << std::dec << "\033[0m" << std::endl;
                        if(verbosity & SV_MODEL) {
                            for (const auto& pair : model_else) {
                                std::cout << "\t" << pair.first << ": "<< pair.second << std::endl;
                            }
                        }
                    }
                    setConcreteRegisterValue(registers.x86_rip, ite[2]->evaluate());
                    cnstrs.push_back(cnstr_else);

                } // sat && sat

            } // found ite

        } // is symbolic branch

    } // exploration loop

    depth--;
    return false;
}


/**
 * Add a hook to an instruction.
 * @param addr - Address of the instruction.
 * @param callback - InsnHook to call after processing the instruction.
 */
void Swimmer::hookInstruction(triton::uint64 addr, InsnHook callback) {
    if(!insnHooks.count(addr))
        insnHooks[addr] = {};
    insnHooks[addr].push_back(callback);
}


/**
 * Add a hook to a function.
 * @param addr - Address of the function.
 * @param callback - FuncHook to call after instead of the function.
 */
void Swimmer::hookFunction(triton::uint64 addr, FuncHook callback) {
    if(!funcHooks.count(addr))
        funcHooks[addr] = {};
    funcHooks[addr].push_back(callback);
}


/**
 * Mark an address as dead, stopping execution if it is reached.
 * @param addr - Dead address
 */
void Swimmer::killAddress(triton::uint64 addr) {
    deadEnds.push_back(addr);
}


/**
 * Define an instruction in execute in lieu of a static instruction.
 * @param addr - Address to inject the instruction.
 * @param insn - Instruction to inject.
 */
void Swimmer::injectInstruction(triton::uint64 addr, triton::arch::Instruction insn) {
    injectedInstructions[addr] = insn;
}


/**
 * Define a jump condition in lieu of the original.
 * @param addr - Address of the jump instruction.
 * @param guard - Logical condition to satisfy the jump.
 * @return if the injection was made.
 */
bool Swimmer::injectJumpCondition(triton::uint64 addr, triton::ast::SharedAbstractNode guard) {
    // Get the instruction bytes
    if(!isConcreteMemoryValueDefined(addr, 1))
        return false;
    std::vector<triton::uint8> opcode = getConcreteMemoryAreaValue(addr, 16);

    // Sanity check the instruction
    triton::arch::Instruction jump = triton::arch::Instruction(addr, opcode.data(), 16);
    disassembly(jump);
    if(!jump.isBranch() || jump.getType() == triton::arch::x86::ID_INS_JMP)
        return false;

    // Parse the original if-then-else for the destinations
    triton::uint64 fallDst = jump.getNextAddress();
    triton::uint64 jumpDst = jump.operands[0].getImmediate().getValue();

    // Create the new if-then-else
    auto ac = getAstContext();
    auto ite = ac->ite(guard, ac->bv(jumpDst, 64), ac->bv(fallDst, 64));
    auto iteExpr = newSymbolicExpression(ite);
    jump.symbolicExpressions = {iteExpr};

    // Inject the instruction to implement the condition
    injectInstruction(addr, jump);
    return true;
}


/**
 * Creates a model for the current constraints.
 * @return a model for the current constraints.
 */
std::unordered_map<long unsigned int, triton::engines::solver::SolverModel> Swimmer::getSatModel() {
    if(cnstrs.size() > 1)
        return getModel(astCtxt->land(cnstrs));
    else if(cnstrs.size() == 1)
        return getModel(cnstrs[0]);
    return {};
}


/**
 * Creates several models for the current constraints.
 * @param limit - The number of models to return
 * @return a vector of models for the current constraints
 */
std::vector<std::unordered_map<long unsigned int, triton::engines::solver::SolverModel>> Swimmer::getSatModels(uint limit) {
    if(cnstrs.size() > 1)
        return getModels(astCtxt->land(cnstrs), limit);
    else if(cnstrs.size() == 1)
        return getModels(cnstrs[0], limit);
    return {};
}


/**
 * Reads a string from memory
 * @param ptr - Address in memory to read from.
 * @return the string stored in memory (if defined).
 */
std::string Swimmer::readString(triton::uint64 ptr) {
    std::stringstream ss;
    unsigned char c = getConcreteMemoryAreaValue(ptr, 1)[0];
    while(c != '\0') {
        if(!isConcreteMemoryValueDefined(ptr))
            break;
        ss << c;
        ptr++;
        c = getConcreteMemoryValue(ptr);
    }
    return ss.str();
}


/**
 * Symbolizes bytes in memory with information on the source.
 * @param id - Identifying name for the memory (fgets, strcpy, etc).
 * @param ptr - Address to be symbolized.
 * @param sink - Address at which the memory was symbolized.
 * @param len - Length of the memory access.
 * @return the SharedSymbolicVariables of the memory.
 */
Buffer Swimmer::symbolizeNamedMemory(std::string id, triton::uint64 ptr, triton::uint64 sink, size_t len) {
    // Create the buffer
    Buffer b = Buffer(id, sink, ptr, len);

    // Create the symbolic variables
    triton::arch::MemoryAccess mem;
    std::stringstream ss;
    for(size_t i = 0; i < len; i++) {
        mem = triton::arch::MemoryAccess(ptr + i, 1);
        ss << b.alias << "[0x" << i << "]";
        b.vars.push_back(symbolizeMemory(mem, ss.str()));
        clearConcreteMemoryValue(mem);
        ss.str(std::string());
    }

    // Return the resulting buffer  
    return b;
}


/**
 * Symbolizes bytes in memory with information on the source.
 * @param id - Identifying name for the memory (fgets, strcpy, etc).
 * @param ptr - Address to be symbolized.
 * @param sink - Address at which the memory was symbolized.
 * @param sz - Size of the memory access.
 * @return the Buffer for the memory
 */
triton::engines::symbolic::SharedSymbolicVariable Swimmer::symbolizeNamedMemoryChunk(std::string id, triton::uint64 ptr, triton::uint64 sink, ushort sz) {
    if(sz == 0)
        return nullptr;
    
    triton::arch::MemoryAccess mem = triton::arch::MemoryAccess(ptr, sz);
    std::stringstream ss;
    ss << id << "<--0x" << std::hex << sink << std::dec;
    return symbolizeMemory(mem, ss.str());
}


/**
 * Allocate a chunk of heap memory.
 * @param id - Identifying name for the memory (fgets, strcpy, etc).
 * @param sink - Address at which the memory was symbolized.
 * @param len - Length of the memory access.
 * @return the address the allocation begins at.
 */
triton::uint64 Swimmer::allocateHeapMemory(std::string id, triton::uint64 sink, size_t len) {
    // Validate length
    if(len == 0) return 0;

    // Start search at beginning of heap
    triton::uint64 ptr = HEAP_START;

    // Find starting address for chunk
    while(ptr < HEAP_END) {
        if(!isConcreteMemoryValueDefined(ptr) && !isMemorySymbolized(ptr) && !isHeapAllocated(ptr, len)) {
            triton::uint64 i;
            for(i = 1; i < len; i++) {
                if(isConcreteMemoryValueDefined(ptr+i) || isMemorySymbolized(ptr+i))
                    break;
            }
            if(i == len) break;
            else ptr += i;
        }
        else ptr += 1;
    }

    // Validate enough memory
    if(ptr + len > HEAP_END) return 0;

    // Create the buffer
    Buffer b = symbolizeNamedMemory(id, ptr, sink, len);
    heapAllocations.emplace(ptr, b);
    if(verbosity & SV_ALLOC) {
        std::cout << "\033[1mAllocated " << len << " bytes @ 0x"
                  << std::hex << ptr << std::dec << "\033[0m" << std::endl;
    }

    // Return the address for allocation
    return ptr;
}


/**
 * Free a chunk of heap memory.
 * @param ptr - Starting address of the chunk.
 * @param sink - Address at which the buffer can be declared free'd.
 * @return true if successfully free'd, false if not allocated or already free'd.
 */
bool Swimmer::freeHeapMemory(triton::uint64 ptr, triton::uint64 sink) {
    for(auto &pair : heapAllocations) {
        if(ptr == pair.first) {
            if(verbosity & SV_ALLOC) {
                std::cout << "\033[1mFreeing pointer @ 0x"
                          << std::hex << ptr << std::dec << "\033[0m" << std::endl;
            }
            return pair.second.kill(sink);
        }
    }
    return false;
}


/**
 * Check if an address of heap memory is live.
 * @param ptr - Address of the chunk
 * @param strict - If true, the address must start a chunk (default=false).
 * @return true if the heap memory at ptr is alive.
 */
bool Swimmer::statHeapMemory(triton::uint64 ptr, bool strict) {
    for(auto &pair : heapAllocations) {
        triton::uint64 lo = pair.first;
        triton::uint64 hi = lo + pair.second.getSize() - 1;
        if(ptr == lo)
            return pair.second.stat();
        if(!strict && ptr <= hi && ptr >= lo) {
            return pair.second.stat();
        }
    }
    return false;
}


/**
 * Get the origin of a pointer in the heap.
 * @param ptr - Address of the pointer.
 * @param strict - If true, the address must start a chunk (default=false)
 * @return the origin of ptr.
 */
triton::uint64 Swimmer::getHeapOrigin(triton::uint64 ptr, bool strict) {
    for(auto &pair : heapAllocations) {
        triton::uint64 lo = pair.first;
        triton::uint64 hi = lo + pair.second.getSize() - 1;
        if(ptr == lo)
            return pair.second.getOrigin();
        if(!strict && ptr <= hi && ptr >= lo) {
            return pair.second.getOrigin();
        }
    }
    return 0;
}


/**
 * Get the origin of a pointer in the heap.
 * @param ptr - Address of the pointer.
 * @param strict - If true, the address must start a chunk (default=false)
 * @return the origin of ptr.
 */
triton::uint64 Swimmer::getHeapSink(triton::uint64 ptr, bool strict) {
    for(auto &pair : heapAllocations) {
        triton::uint64 lo = pair.first;
        triton::uint64 hi = lo + pair.second.getSize() - 1;
        if(ptr == lo)
            return pair.second.getSink();
        if(!strict && ptr <= hi && ptr >= lo) {
            return pair.second.getSink();
        }
    }
    return 0;
}


/**
 * Check if any bytes in a heap memory range has been allocated.
 * The state of the memory is not checked.
 * @param ptr - Start of memory range.
 * @param len - Length of memory range.
 * @returns true if any bytes [ptr, ptr+len-1] have been allocated.
 */
bool Swimmer::isHeapAllocated(triton::uint64 ptr, size_t len) {
    for(auto &pair : heapAllocations) {
        triton::uint64 lo = pair.first;
        triton::uint64 hi = lo + pair.second.getSize() - 1;
        if(ptr <= hi && ptr + len - 1 >= lo)
            return true;
    }
    return false;
}


/**
 * Get the length of an allocation on the heap
 * @param ptr - Address of the allocation.
 * @return the length of allocation, including 0 for no allocation.
 */
size_t Swimmer::getAllocatedLength(triton::uint64 ptr) {
    auto it = heapAllocations.find(ptr);
    if(it != heapAllocations.end())
        return it->second.getSize();
    return 0;
}


/**
 * Get the alias of a Buffer
 * @param ptr - Address of the buffer.
 * @return the alias of the buffer, or UNDEFINED if it doesn't exist
 */
std::string Swimmer::getBufferAlias(triton::uint64 ptr) {
    auto it = heapAllocations.find(ptr);
    if(it != heapAllocations.end())
        return it->second.alias;
    return "UNDEFINED";
}


/**
 * Get the deduced length of a buffer on the stack.
 * This is a "best guess" that uses stackframe size and accesses.
 * @param ptr - Beginning of the buffer.
 * @return the deduced length of the buffer at ptr on the stack.
 */
size_t Swimmer::getStackBufferLength(triton::uint64 ptr) {
    for(Stackframe &sf : stackframes) {
        triton::uint64 hi = sf.getAddress();
        triton::uint64 lo = hi - sf.getSize();
        if(ptr <= hi && ptr >= lo) {
            ptr = hi - ptr;
            return sf.getAccessGap(ptr);
        }
    }
    return 0;
}


/**
 * Get the current stackframe.
 * @return the current stackframe, or nullptr if there is no frame.
 */
Stackframe *Swimmer::getStackframe() {
    if(stackframes.size() == 0)
        return nullptr;
    return &stackframes.back();
}


/**
 * Get the stackframe containing an addresss.
 * @param ptr - Address that the stackframe should contain.
 * @return the stackframe containing ptr, or nullptr if there is no frame.
 */
Stackframe *Swimmer::getStackframe(triton::uint64 ptr) {
    for(Stackframe &sf : stackframes) {
        triton::uint64 hi = sf.getAddress();
        triton::uint64 lo = hi - sf.getSize();
        if(ptr <= hi && ptr >= lo) {
            ptr = hi - ptr;
            return &sf;
        }
    }
    return nullptr;
}


/**
 * Check if an address belongs to the stack.
 * @param ptr - Address to check.
 * @return true if the address is within the stack.
 */
bool Swimmer::isStackAddress(triton::uint64 ptr) {
    if(stackframes.size() == 0)
        return false;
    return ptr >= STACK_END && ptr <= STACK_START;
}


/**
 * Check if an address belongs to the heap.
 * @param ptr - Address to check
 * @return true if the address is within the heap address space.
 */
bool Swimmer::isHeapAddress(triton::uint64 ptr) {
    return ptr >= HEAP_START && ptr <= HEAP_END;
}



/*********************/
/* PRIVATE FUNCTIONS */
/*********************/


/**
 * Handle changing of the stack pointer to allocate the stackframe
 * @param insn - Potential instruction to perform the change.
 * @return true if the stackframe was allocated
 */
bool Swimmer::__handleStackAllocation(triton::arch::Instruction insn) {
    if(insn.getType() == triton::arch::x86::ID_INS_SUB && insn.operands[0] == registers.x86_rsp) {
        // Update the stackframe
        triton::uint64 base = triton::uint64(getConcreteRegisterValue(registers.x86_rbp));
        size_t sz = insn.operands[1].getImmediate().getValue();
        if(sz > 0xFF00000000000000)
            return false;
        stackframes.back().update(base, sz);

        // Create the symbolic variables
        triton::arch::MemoryAccess mem;
        std::stringstream ss;
        for(size_t i = 0; i < sz; i++) {
            mem = triton::arch::MemoryAccess(base - i, 1);
            ss << "stackframe@0x" << std::hex
               << insn.getAddress() << "_0x" << std::hex
               << base << "[-0x" << i << "]";
            symbolizeMemory(mem, ss.str());
            clearConcreteMemoryValue(mem);
            ss.str(std::string());
        }

        if(verbosity & SV_STACK) {
            std::cout << "\033[1mIdentified stackframe @ 0x"
                      << std::hex << base << " has 0x"
                      << sz << " bytes\033[0m" << std::dec << std::endl;
        }

        return true;
    }
    return false;
}


/**
 * Handle accessing of the stack
 * @param insn - Potential instruction to perform access.
 * @return true if the stackframe was accessed
 */
bool Swimmer::__handelStackReference(triton::arch::Instruction insn) {
    // A stack reference has two operands
    if(insn.operands.size() == 2) {
        bool refFound = false;
        for(auto &op : insn.operands) {

            // A stack reference operand is memory based on RBP
            if(op.getType() == triton::arch::OP_MEM) {
                triton::arch::MemoryAccess memSrc = op.getMemory();
                if(memSrc.getBaseRegister() == registers.x86_rbp) {

                    // Note the displacement as an access
                    triton::uint64 disp = -memSrc.getDisplacement().getValue();
                    bool newAccess = stackframes.back().addAccess(disp);
                    refFound = true;

                    if(newAccess && verbosity & SV_STACK) {
                        std::cout << "\033[1mNew access to stackframe @ 0x" << std::hex
                                  << disp << "\033[0m" << std::endl;
                    }
                }
            }
        }
        return refFound;
    }
    return false;
}


/**
 * Handle CALL instructions, respecting unknown memory and hooks
 * @param pc - Address of the call instruction
 * @param localFid - Fork ID of the swimmer
 * @param insn - Triggering instruction
 * @return if the call was unknown or hooked
 */
bool Swimmer::__handleCall(uint localFid, triton::uint64 pc, triton::arch::Instruction insn) {
    // Verify the instruction is a call
    if(insn.getType() == triton::arch::x86::ID_INS_CALL) {
        triton::uint64 dst = triton::uint64(getConcreteRegisterValue(registers.x86_rip));
        bool isHooked = funcHooks.count(dst);
        bool isUndefined = !isConcreteMemoryValueDefined(dst, 1);
        if(isHooked || isUndefined) {
            // Perform function hooks
            if(isHooked) {
                if(verbosity & SV_INSN)
                    std::cout << "[" << localFid << "] " << "(" << depth << ") "
                              << "0x------  [func hook]" << std::endl;
                for(FuncHook& callback : funcHooks[dst]) {
                    triton::uint64 retVal = callback(this, pc);
                    setConcreteRegisterValue(registers.x86_rax, retVal);
                }
            }
            // Just step over
            else if (verbosity & SV_INSN)
                std::cout << "[" << localFid << "] " << "(" << depth << ") "
                            << "0x------  [step over]" << std::endl;
            triton::uint512 correctedRsp = getConcreteRegisterValue(registers.x86_rsp) + 8;
            setConcreteRegisterValue(registers.x86_rsp, correctedRsp);
            setConcreteRegisterValue(registers.x86_rip, insn.getNextAddress());
        } else {
            stackframes.push_back(Stackframe());
        }
        return isHooked || isUndefined;
    }
    return false;
}


/**
 * Symbolize memory when reading or loading from an undefined address
 * @param pc - Address of the reading instruction.
 * @param insn - Triggering instruction.
 * @return if the memory was symbolized.
 */
bool Swimmer::__handleMemoryRead(triton::uint64 pc, triton::arch::Instruction insn) {
    // Instruction is a candidate memory read
    if(insn.isMemoryRead() && insn.operands.size() == 2 && insn.operands[1].getType() == triton::arch::OP_MEM) {
        // Check if the memory access is one we care about
        triton::arch::MemoryAccess memSrc = insn.operands[1].getMemory();
        bool diffSeg = isRegister(memSrc.getSegmentRegister());
        bool defined = isConcreteMemoryValueDefined(memSrc.getAddress());
        bool symbolized = isMemorySymbolized(memSrc);

        // Not defined, symbolized, nor in a different segment
        if(!defined && !symbolized && !diffSeg) {
            std::stringstream ss;
            ss << "stackMem<--0x" << std::hex << pc << std::dec;
            symbolizeMemory(memSrc, ss.str());
            processing(insn);
            return true;
        }

        return false;
    }

    // Instruction is a LEA
    else if(insn.getType() == triton::arch::x86::ID_INS_LEA && insn.operands[1].getType() == triton::arch::OP_MEM) {
        // Check if the memory load is from the stack
        triton::arch::MemoryAccess memSrc = insn.operands[1].getMemory();
        triton::arch::Register& base = memSrc.getBaseRegister();
        bool fromRbp = (base == registers.x86_rbp);
        bool defined = isConcreteMemoryValueDefined(memSrc);
        bool symbolized = isMemorySymbolized(memSrc);

        // New memory, displaced from base pointer
        // NOTE: Displacement from stack pointer causes weird behavior
        if(fromRbp && !defined && !symbolized) {
            std::stringstream ss;
            ss << "stackMem<--0x" << std::hex << pc << std::dec;
            symbolizeMemory(memSrc, ss.str());
            processing(insn);
        }
        return fromRbp && !defined && !symbolized;
    }
    return false;
}


/**
 * Print the values of concrete registers and note symbolic registers
 * @param all - Print all registers, not only general purpose
 */
void Swimmer::__printRegisters(bool all) {
    // All printing in this function should be hexadecimal
    std::cout << std::hex;

    // General purpose registers
    triton::arch::Register generalPurpose[] = { registers.x86_rax, registers.x86_rbx
                                              , registers.x86_rcx, registers.x86_rdx
                                              , registers.x86_rsi, registers.x86_rdi
                                              , registers.x86_rbp, registers.x86_rsp };

    for(auto &reg : generalPurpose) {
        // Register is symbolic, not much to print
        if(isRegisterSymbolized(reg)) {
            std::cout << "\t" << "\033[34m" << reg.getName() << "\033[0m" << std::endl;
        }

        // Register is concrete
        else {
            std::cout << "\t" << "\033[90m" << reg.getName();

            // Print the value in the register
            triton::uint64 val = triton::uint64(getConcreteRegisterValue(reg));
            std::cout << " = 0x" << val;

            // Continually print memory pointers until not concrete
            triton::arch::MemoryAccess ptr = triton::arch::MemoryAccess(val, 8);
            while(!isMemorySymbolized(ptr) && isConcreteMemoryValueDefined(ptr)) {
                val = triton::uint64(getConcreteMemoryValue(ptr));
                if(val == ptr.getAddress()) break;
                std::cout << " -> 0x" << val;
                ptr = triton::arch::MemoryAccess(val, 8);
            }

            // Print final memory pointer if symbolic
            if(isMemorySymbolized(ptr)) {
                std::cout << "\033[34m -> *";
            }

            // Restore text formatting
            std::cout << "\033[0m" << std::endl;
        }
    }

    // Restore printing to decimal
    std::cout << std::dec;
    return;
}
