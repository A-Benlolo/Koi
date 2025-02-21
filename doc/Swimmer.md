# swimmer.h

## Public

### Public Class Members

```cpp
std::vector<triton::ast::SharedAbstractNode> cnstrs;
```
Vector storing constraints for the current execution path.


```cpp
SV_FLAG verbosity = 0;
```
Current verbosity level.


```cpp
static const SV_FLAG SV_INSN = 0b00000001; // Print instructions at each step
static const SV_FLAG SV_SYMS = 0b00000010; // Print symbols at each step (not yet implemented)
static const SV_FLAG SV_REGS = 0b00000100; // Print register state/value at each step
static const SV_FLAG SV_BRANCH = 0b00001000; // Print when a branch jump or fallthrough is taken
static const SV_FLAG SV_MODEL = 0b00010000; // Print the model when at branch points
static const SV_FLAG SV_STOPS = 0b00100000; // Print reason for ending path exploration
static const SV_FLAG SV_ALLOC = 0b01000000; // Print when memory is heap-allocated
static const SV_FLAG SV_STACK = 0b10000000; // Print when the stackframe is accessed
static const SV_FLAG SV_NONE = 0b00000000; // Print nothing
static const SV_FLAG SV_ALL = 0b11111111; // Print everything
static const SV_FLAG SV_CTRLFLOW = SV_INSN | SV_BRANCH | SV_STOPS; // Print control flow data
static const SV_FLAG SV_MEM = SV_ALLOC | SV_STACK; // Print memory data
```



### Public Functions

#### Constructors

```cpp
Swimmer(const std::string& filein);
```
Constructor that initializes the swimmer with the provided file.
- `filein`: Path to the file to load.
Returns a new `Swimmer` object.



#### Setters

```cpp
void setPc(triton::uint64 x);
```
Sets the value of the instruction pointer (PC).
- `x`: Value to set the instruction pointer to.



#### Getters

```cpp
std::unordered_map<long unsigned int, triton::engines::solver::SolverModel> getSatModel();
```
Creates a model for the current constraints.
Returns a model for the current constraints.


```cpp
std::vector<std::unordered_map<long unsigned int, triton::engines::solver::SolverModel>> getSatModels(uint limit);
```
Creates several models for the current constraints.
- `limit`: The number of models to return.
Returns a vector of models for the current constraints.


```cpp
std::string readString(triton::uint64 ptr);
```
Reads a string from memory at the specified address.
- `ptr`: Address in memory to read from.
Returns the string stored at the specified memory address (if defined).



#### Control Flow, Injection, Hooking

```cpp
bool explore(triton::uint64 target=0, uint maxVisits=0, uint maxDepth=0);
```
Explores the memory pool from the instruction pointer, respecting hooks and injections.
- `target`: Desired address to execute (default is `0`).
- `maxVisits`: Maximum number of visits to the same instruction (default is `0`).
- `maxDepth`: Maximum fork depth (default is `0`).
Returns `true` if the target address was reached, otherwise `false`.


```cpp
void hookInstruction(triton::uint64 addr, InsnHook callback);
```
Adds a hook to an instruction at the specified address.
- `addr`: Address of the instruction to hook.
- `callback`: Function to call after processing the instruction.


```cpp
void hookFunction(triton::uint64 addr, FuncHook callback);
```
Adds a hook to a function at the specified address.
- `addr`: Address of the function to hook.
- `callback`: Function to call after processing the function.


```cpp
void killAddress(triton::uint64 addr);
```
Marks an address as "dead" and stops execution if it is reached.
- `addr`: Address to mark as dead.


```cpp
void injectInstruction(triton::uint64 addr, triton::arch::Instruction insn);
```
Injects an instruction to execute at the specified address.
- `addr`: Address to inject the instruction.
- `insn`: Instruction to inject.


```cpp
bool injectJumpCondition(triton::uint64 addr, triton::ast::SharedAbstractNode guard);
```
Defines a jump condition in lieu of the original.
- `addr`: Address of the jump instruction.
- `guard`: Logical condition that must be satisfied for the jump.
Returns `true` if the injection was successful, otherwise `false`.



#### Memory Management


```cpp
Buffer symbolizeNamedMemory(std::string id, triton::uint64 ptr, triton::uint64 sink, triton::uint64 len);
```
Symbolizes bytes in memory with information on the source.
- `id`: Identifying name for the memory (e.g., `fgets`, `strcpy`).
- `ptr`: Address to symbolize.
- `sink`: Address at which the memory was symbolized.
- `len`: Length of the memory access.
Returns the Buffer for the memory.


```cpp
triton::engines::symbolic::SharedSymbolicVariable symbolizeNamedMemoryChunk(std::string id, triton::uint64 ptr, triton::uint64 sink, ushort sz);
```
Symbolizes bytes in memory with information on the source, considering a chunk of memory.
- `id`: Identifying name for the memory (e.g., `fgets`, `strcpy`).
- `ptr`: Address to symbolize.
- `sink`: Address at which the memory was symbolized.
- `sz`: Size of the memory access.
Returns the SharedSymbolicVariable of the memory.


```cpp
triton::uint64 Swimmer::allocateHeapMemory(std::string id, triton::uint64 sink, size_t len);
```
Allocate a chunk of heap memory.
- `id` - Identifying name for the memory (fgets, strcpy, etc).
- `sink` - Address at which the memory was symbolized.
- `len` - Length of the memory access.
Returns the address the allocation begins at.


```cpp
bool freeHeapMemory(triton::uint64 ptr);
```
Free a chunk of heap memory.
- `ptr`: Starting address of the chunk.
- Returns true if successfully free'd, false if not allocated or already free'd.


```cpp
bool getHeapOrigin(triton::uint64 ptr, bool strict=false)
```
Get the origin of a pointer in the heap.
- `ptr` - Address of the chunk
- `strict` - If true, the address must start a chunk (default=false).
Returns true if the heap memory at ptr is alive.


```cpp
bool getHeapSink(triton::uint64 ptr, bool strict=false)
```
Get the sink of a pointer in the heap.
- `ptr` - Address of the chunk
- `strict` - If true, the address must start a chunk (default=false).
Returns true if the heap memory at ptr is alive.


```cpp
bool statHeapMemory(triton::uint64 ptr, bool strict=false)
```
Check if an address of heap memory is live.
- `ptr` - Address of the chunk
- `strict` - If true, the address must start a chunk (default=false).
Returns true if the heap memory at ptr is alive.


```cpp
bool isHeapAllocated(triton::uint64 ptr, size_t len);
```
Check if any bytes in a memory range has been allocated. The state of the memory is not checked.
- `ptr`: Start of memory range.
- `len`: Length of memory range.
Returns true if any bytes \[ptr, ptr+len-1\] have been allocated.


```cpp
bool isHeapStub(triton::uint64 ptr);
```
Check if an address points to a heap allocation.
- `ptr`: Address to check.
Returns true if the address points to a heap allocation.


```cpp
triton::uint64 getHeapStub(triton::uint64 ptr);
```
Get the heap pointer that owns an address.
- `ptr`: Address to query.
Returns the owning heap address, including 0 for no owner.


```cpp
size_t getAllocatedLength(triton::uint64 ptr);
```
Get the length of an allocation on the heap
- `ptr`: Address of the allocation.
Return the length of allocation, including 0 for no allocation.


```cpp
std::string getBufferAlias(triton::uint64 ptr);
```
Get the alias of a Buffer
- `ptr` - Address of the buffer.
Returns the alias of the buffer, or "UNDEFINED" if it doesn't exist.


```cpp
size_t getStackBufferLength(triton::uint64 ptr);
```
Get the deduced length of a buffer on the stack. This is a "best guess" that uses stackframe size and accesses.
- `ptr` - Beginning of the buffer.
Returns the deduced length of the buffer at ptr on the stack.


```cpp
Stackframe *getStackframe();
Stackframe *getStackframe(triton::uint64 ptr);
```
Get the current stackframe or stackframe containing an address.
- `ptr` - Address that the stackframe should contain.
Returns the size of the current or requested stackframe, or nullptr if there is no frame.


```cpp
bool isStackAddress(triton::uint64 ptr);
```
Check if an address belongs to the stack.
- `ptr` Address to check.
Returns true if the address is within the stack.


```cpp
bool isHeapAddress(triton::uint64 ptr);
```
Check if an address belongs to the heap.
- `ptr` Address to check.
Returns true if the address is within the heap.





## Private

### Private Class Members

```cpp
static const uint STACK_START = 0x7ffffffe; // Starting stack address
static const uint STACK_END   = 0x70000000; // Ending stack address (grows down)
static const uint HEAP_START = 0x1000000; // Starting heap address
static const uint HEAP_END   = 0x2000000; // Ending heap address (grows up)
```

```cpp
typedef void (*InsnHook)(Swimmer*, triton::arch::Instruction);
```
Typedef for a function pointer to a hook function that is called after processing an instruction.


```cpp
triton::uint64 (*FuncHook)(Swimmer*, triton::uint64);
```
Typedef for a function pointer to a hook function that is called instead of processing a function. The return value is stored in RAX.


```cpp
typedef unsigned char SV_FLAG;
```
Typedef for a flag type to define verbosity levels.


```cpp
std::vector<triton::uint64> deadEnds;
```
Vector storing addresses marked as "dead" that stop execution when reached.


```cpp
uint depth = 0;
```
Current depth of the execution branch.


```cpp
uint fid = 0;
```
Current fork ID of the swimmer.



```cpp
std::unordered_map<triton::uint64, std::vector<FuncHook>> funcHooks;
```
Map associating function addresses with a list of function hooks.


```cpp
std::unordered_map<triton::uint64, Buffer> heapAllocations;
```
Map associating addresses with a heap-based buffer.


```cpp
std::unordered_map<triton::uint64, triton::arch::Instruction> injectedInstructions;
```
Map associating instruction addresses with the injected instructions.


```cpp
std::unordered_map<triton::uint64, std::vector<InsnHook>> insnHooks;
```
Map associating instruction addresses with a list of instruction hooks.


```cpp
std::vector<Stackframe> stackframes
```
Vector tracking the stackframe of the function call stack


```cpp
std::unordered_map<triton::uint64, uint> visits;
```
Map associating instruction addresses with the number of visits.



### Private Functions


```cpp
bool __handleStackAllocation(triton::arch::Instruction insn);
```
Handle changing of the stack pointer to allocate the stackframe
- `insn`: Potential instruction to perform the change.
Returns true if the stackframe was allocated


```cpp
bool __handelStackReference(triton::arch::Instruction insn);
```
Handle accessing of the stack
- `insn`: Potential instruction to perform access.
Returns true if the stackframe was accessed


```cpp
bool __handleCall(uint localFid, triton::uint64 pc, triton::arch::Instruction insn);
```
Handle CALL instructions, respecting unknown memory and hooks.
- `pc`: Address of the call instruction.
- `localFid`: Fork ID of the swimmer.
- `insn`: Triggering instruction.
Returns if the call was unknown or hooked.


```cpp
bool __handleMemoryRead(triton::uint64 pc, triton::arch::Instruction insn);
```
Symbolizes memory when reading or loading from an undefined address.
- `pc`: Address of the reading instruction.
- `insn`: Triggering instruction.
Returns if the memory was symbolized.


```cpp
void __printRegisters(bool all=false);
```
Prints the values of concrete registers and notes symbolic registers.
- `all`: If `true`, prints all registers, not just general-purpose ones (default is `false`).
