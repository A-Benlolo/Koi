#ifndef SWIMMER_H
#define SWIMMER_H

#include <triton/context.hpp>
#include <optional>
#include "Koi/buffer.h"
#include "Koi/stackframe.h"


class Swimmer: public triton::Context {
private:
    /* Ease of use constants (temporary) */
    static const uint STACK_START = 0x7ffffffe;
    static const uint STACK_END   = 0x70000000;
    static const uint HEAP_START = 0x1000000;
    static const uint HEAP_END   = 0x2000000;


    /* Hook typedefs */
    typedef void (*InsnHook)(Swimmer*, triton::arch::Instruction);
    typedef triton::uint64 (*FuncHook)(Swimmer*, triton::uint64);


    /* Verbosity typedef */
    typedef unsigned char SV_FLAG;


    /* New class members */
    std::vector<triton::uint64> deadEnds;
    uint depth = 0;
    uint fid = 0;
    std::unordered_map<triton::uint64, std::vector<FuncHook>> funcHooks;
    std::unordered_map<triton::uint64, Buffer> heapAllocations;
    std::unordered_map<triton::uint64, triton::arch::Instruction> injectedInstructions;
    std::unordered_map<triton::uint64, std::vector<InsnHook>> insnHooks;
    std::vector<Stackframe> stackframes;
    std::unordered_map<triton::uint64, uint> visits;


    /**
     * Handle changing of the stack pointer to allocate the stackframe
     * @param insn - Potential instruction to perform the change.
     * @return true if the stackframe was allocated
     */
    bool __handleStackAllocation(triton::arch::Instruction insn);


    /**
     * Handle accessing of the stack
     * @param insn - Potential instruction to perform access.
     * @return true if the stackframe was accessed
     */
    bool __handelStackReference(triton::arch::Instruction insn);

    /**
     * Handle CALL instructions, respecting unknown memory and hooks and injections
     * @param pc - Address of the call instruction
     * @param localFid - Fork ID of the swimmer
     * @param insn - Triggering instruction
     * @return if the call was unknown or hooked
     */
    bool __handleCall(uint localFid, triton::uint64 pc, triton::arch::Instruction insn);


    /**
     * Symbolize memory when reading or loading from an undefined address
     * @param pc - Address of the reading instruction.
     * @param insn - Triggering instruction.
     * @return if the memory was symbolized.
     */
    bool __handleMemoryRead(triton::uint64 pc, triton::arch::Instruction insn);


    /**
     * Print the values of concrete registers and note symbolic registers
     * @param all - Print all registers, not only general purpose.
     */
    void __printRegisters(bool all=false);


public:
    /* Static verbosity flags */
    static const SV_FLAG SV_INSN   = 0b00000001;
    static const SV_FLAG SV_SYMS   = 0b00000010;
    static const SV_FLAG SV_REGS   = 0b00000100;
    static const SV_FLAG SV_BRANCH = 0b00001000;
    static const SV_FLAG SV_MODEL  = 0b00010000;
    static const SV_FLAG SV_STOPS  = 0b00100000;
    static const SV_FLAG SV_ALLOC  = 0b01000000;
    static const SV_FLAG SV_STACK  = 0b10000000;
    static const SV_FLAG SV_NONE   = 0b00000000;
    static const SV_FLAG SV_ALL    = 0b11111111;
    static const SV_FLAG SV_CTRLFLOW = SV_INSN | SV_BRANCH | SV_STOPS;
    static const SV_FLAG SV_MEM    = SV_ALLOC | SV_STACK;


    /* New class members */
    std::vector<triton::ast::SharedAbstractNode> cnstrs;
    SV_FLAG verbosity = 0;


     /**
     * Constructor
     * @param filein - Path to a file to load.
     * @return a new Swimmer
     */
    Swimmer(const std::string& filein);


    /**
     * Set the instruction pointer (pc) value.
     * @param x - Value to set to.
     */
    void setPc(triton::uint64 x);


    /**
     * Explores the memory pool from the instruction pointer, respecting hooks
     * @param target - Desired address to execute
     * @param maxVisits - Maximum number of times to execute the same instruction
     * @param maxDepth - Maximum fork depth of an execution branch
     * @return if the target was reached (default=False)
     */
    bool explore(triton::uint64 target=0, uint maxVisits=0, uint maxDepth=0);


    /**
     * Add a hook to an instruction.
     * @param addr - Address of the instruction.
     * @param callback - InsnHook to call after processing the instruction.
     */
    void hookInstruction(triton::uint64 addr, InsnHook callback);


    /**
     * Add a hook to a function.
     * @param addr - Address of the function.
     * @param callback - FuncHook to call after instead of the function.
     */
    void hookFunction(triton::uint64 addr, FuncHook callback);


    /**
     * Mark an address as dead, stopping execution if it is reached.
     * @param addr - Dead address
     */
    void killAddress(triton::uint64 addr);


     /**
      * Define an instruction in execute in lieu of a static instruction.
      * @param addr - Address to inject the instruction.
      * @param insn - Instruction to inject.
      */
    void injectInstruction(triton::uint64 addr, triton::arch::Instruction insn);


    /**
     * Define a jump condition in lieu of the original.
     * @param addr - Address of the jump instruction.
     * @param guard - Logical condition to satisfy the jump.
     * @return if the injection was made.
     */
    bool injectJumpCondition(triton::uint64 addr, triton::ast::SharedAbstractNode guard);


    /**
     * Creates a model for the current constraints.
     * @return a model for the current constraints.
     */
    std::unordered_map<long unsigned int, triton::engines::solver::SolverModel> getSatModel();


    /**
     * Creates several models for the current constraints.
     * @param limit - The number of models to return
     * @return a vector of models for the current constraints
     */
    std::vector<std::unordered_map<long unsigned int, triton::engines::solver::SolverModel>> getSatModels(uint limit);


    /**
     * Reads a string from memory
     * @param ptr - Address in memory to read from.
     * @return the string stored in memory (if defined).
     */
    std::string readString(triton::uint64 ptr);


    /**
     * Symbolizes bytes in memory with information on the source.
     * @param id - Identifying name for the memory (fgets, strcpy, etc).
     * @param ptr - Address to be symbolized.
     * @param sink - Address at which the memory was symbolized.
     * @param len - Length of the memory access.
     * @return the Buffer for the memory.
     */
    Buffer symbolizeNamedMemory(std::string id, triton::uint64 ptr, triton::uint64 sink, size_t len);


    /**
     * Symbolizes bytes in memory with information on the source.
     * @param id - Identifying name for the memory (fgets, strcpy, etc).
     * @param ptr - Address to be symbolized.
     * @param sink - Address at which the memory was symbolized.
     * @param sz - Size of the memory access.
     * @return the SharedSymbolicVariable of the memory.
     */
    triton::engines::symbolic::SharedSymbolicVariable symbolizeNamedMemoryChunk(std::string id, triton::uint64 ptr, triton::uint64 sink, ushort sz);


    /**
     * Allocate a chunk of heap memory.
     * @param id - Identifying name for the memory (fgets, strcpy, etc).
     * @param sink - Address at which the memory was symbolized.
     * @param len - Length of the memory access.
     * @return the address the allocation begins at.
     */
     triton::uint64 allocateHeapMemory(std::string id, triton::uint64 sink, size_t len);


    /**
     * Free a chunk of heap memory.
     * @param ptr - Starting address of the chunk.
     * @param sink - Address at which the buffer can be declared free'd.
     * @return true if successfully free'd, false if not allocated or already free'd.
     */
    bool freeHeapMemory(triton::uint64 ptr, triton::uint64 sink);


    /**
     * Get the origin of a pointer in the heap.
     * @param ptr - Address of the pointer.
     * @param strict - If true, the address must start a chunk (default=false)
     * @return the origin of ptr.
     */
    triton::uint64 getHeapOrigin(triton::uint64 ptr, bool strict=false);


    /**
    * Get the sink of a pointer in the heap.
    * @param ptr - Address of the pointer.
    * @param strict - If true, the address must start a chunk (default=false)
    * @return the origin of ptr.
    */
    triton::uint64 getHeapSink(triton::uint64 ptr, bool strict=false);


    /**
     * Check if an address of heap memory is live.
     * @param ptr - Address of the chunk
     * @param strict - If true, the address must start a chunk (default=false).
     * @return if the heap memory at ptr is alive.
     */
    bool statHeapMemory(triton::uint64 ptr, bool strict=false);


    /**
     * Check if any bytes in a memory range has been allocated
     * @param ptr - Start of memory range.
     * @param len - Length of memory range.
     * @returns true if any bytes [ptr, ptr+len-1] have been allocated.
     */
    bool isHeapAllocated(triton::uint64 ptr, size_t len);
    

    /**
     * Get the length of an allocation on the heap
     * @param ptr - Address of the allocation.
     * @return the length of allocation, including 0 for no allocation.
     */
    size_t getAllocatedLength(triton::uint64 ptr);


    /**
     * Get the alias of a Buffer
     * @param ptr - Address of the buffer.
     * @return the alias of the buffer, or UNDEFINED if it doesn't exist
     */
    std::string getBufferAlias(triton::uint64 ptr);


    /**
     * Get the deduced length of a buffer on the stack.
     * This is a "best guess" that uses stackframe size and accesses.
     * @param ptr - Beginning of the buffer.
     * @return the deduced length of the buffer at ptr on the stack.
     */
    size_t getStackBufferLength(triton::uint64 ptr);


    /**
     * Get the current stackframe.
     * @return the current stackframe, or nullptr if there is no frame.
     */
    Stackframe *getStackframe();


    /**
     * Get the stackframe containing an addresss.
     * @param ptr - Address that the stackframe should contain.
     * @return the size of the requested stackframe, or nullptr if there is no frame.
     */
    Stackframe *getStackframe(triton::uint64 ptr);


    /**
     * Check if an address belongs to the current stackframe.
     * @param ptr - Address to check.
     * @return true if the address is within the current stackframe.
     */
    bool isStackAddress(triton::uint64 ptr);


    /**
     * Check if an address belongs to the heap.
     * @param ptr - Address to check
     * @return true if the address is within the heap address space.
     */
    bool isHeapAddress(triton::uint64 ptr);
};

#endif
