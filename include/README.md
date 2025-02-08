# Swimmer.h

## Public

```cpp
static const SV_FLAG SV_INSN = 0b00000001;
```
Verbosity flag for printing instructions at each step.


```cpp
static const SV_FLAG SV_SYMS = 0b00000010;
```
Verbosity flag for printing symbols at each step (not implemented).


```cpp
static const SV_FLAG SV_REGS = 0b00000100;
```
Verbosity flag for printing register values at each step.


```cpp
static const SV_FLAG SV_BRANCH = 0b00001000;
```
Verbosity flag for printing when a branch jump or fall through is taken.


```cpp
static const SV_FLAG SV_MODEL = 0b00010000;
```
Verbosity flag for printing the model when a branch jump or fall through is taken.


```cpp
static const SV_FLAG SV_STOPS = 0b00100000;
```
Verbosity flag for printing when a path has been stopped.


```cpp
static const SV_FLAG SV_NONE = 0b00000000;
```
Verbosity flag indicating no verbosity.


```cpp
static const SV_FLAG SV_ALL = 0b11111111;
```
Verbosity flag for enabling all verbosity options.


```cpp
static const SV_FLAG SV_CTRLFLOW = SV_INSN | SV_BRANCH | SV_STOPS;
```
Verbosity flag for control flow information (INSN, BRANCH, STOPS).


```cpp
std::vector<triton::ast::SharedAbstractNode> cnstrs;
```
Vector storing constraints for the current execution.


```cpp
SV_FLAG verbosity = 0;
```
Current verbosity level.


```cpp
Swimmer(const std::string& filein);
```
Constructor that initializes the swimmer with the provided file.
- `filein`: Path to the file to load.
Returns a new `Swimmer` object.


```cpp
void setPc(triton::uint64 x);
```
Sets the value of the instruction pointer (PC).
- `x`: Value to set the instruction pointer to.


```cpp
bool explore(triton::uint64 target=0, uint maxVisits=0, uint maxDepth=0);
```
Explores the memory pool from the instruction pointer, respecting hooks.
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


```cpp
void symbolizeNamedMemory(std::string id, triton::uint64 ptr, triton::uint64 sink, triton::uint64 len);
```
Symbolizes bytes in memory with information on the source.
- `id`: Identifying name for the memory (e.g., `fgets`, `strcpy`).
- `ptr`: Address to symbolize.
- `sink`: Address at which the memory was symbolized.
- `len`: Length of the memory access.


```cpp
void symbolizeNamedMemoryChunk(std::string id, triton::uint64 ptr, triton::uint64 sink, ushort sz);
```
Symbolizes bytes in memory with information on the source, considering a chunk of memory.
- `id`: Identifying name for the memory (e.g., `fgets`, `strcpy`).
- `ptr`: Address to symbolize.
- `sink`: Address at which the memory was symbolized.
- `sz`: Size of the memory access.


---


## Private

```cpp
typedef void (*InsnHook)(Swimmer*, triton::arch::Instruction);
```
Typedef for a function pointer to a hook function that is called after processing an instruction.


```cpp
typedef void (*FuncHook)(Swimmer*, triton::uint64);
```
Typedef for a function pointer to a hook function that is called after processing a function.


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
std::unordered_map<triton::uint64, triton::arch::Instruction> injectedInstructions;
```
Map associating instruction addresses with the injected instructions.



```cpp
std::unordered_map<triton::uint64, std::vector<InsnHook>> insnHooks;
```
Map associating instruction addresses with a list of instruction hooks.



```cpp
std::unordered_map<triton::uint64, uint> visits;
```
Map associating instruction addresses with the number of visits.



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
