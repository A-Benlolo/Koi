#include <algorithm>
#include <triton/context.hpp>
#include "Koi/Bait/common.h"
#include "Koi/Bait/alloc.h"


/**
 * A function hook to robustly allocate heap memory with calloc.
 * @param s - Swimmer responsible for allocation.
 * @param addr - Address that called the original function.
 */
triton::uint64 koi_calloc(Swimmer *s, triton::uint64 addr) {
    // Get and validate the number of elements to allocate for
    triton::uint64 cnt = __getSatisfiableRegisterValue(s, s->registers.x86_rdi);
    if(cnt == 0) return 0;

    // Get and validate the size of elements to allocate for
    triton::uint64 sz = __getSatisfiableRegisterValue(s, s->registers.x86_rsi);
    if(sz == 0) return 0;

    // Allocate and symbolize an available memory chunk
    // No need to zero-out due to heap allocation method
    triton::uint64 ptr = s->allocateHeapMemory("calloc", addr, cnt*sz);

    // Return the address of the new pointer
    return ptr;
}


/**
 * A function hook to robustly free heap memory.
 * @param s - Swimmer responsible for freeing.
 * @param addr - Address that called the original function.
 */
triton::uint64 koi_free(Swimmer *s, triton::uint64 addr) {
    s->freeHeapMemory(__getSatisfiableRegisterValue(s, s->registers.x86_rdi), addr);
    return 0;
}


/**
 * A function hook to robustly allocate heap memory with malloc.
 * @param s - Swimmer responsible for allocation.
 * @param addr - Address that called the original function.
 */
triton::uint64 koi_malloc(Swimmer *s, triton::uint64 addr) {
    // Get and validate the length of allocation
    triton::uint64 len = __getSatisfiableRegisterValue(s, s->registers.x86_rdi);
    if(len == 0) return 0;

    // Allocate and symbolize an available memory chunk
    triton::uint64 ptr = s->allocateHeapMemory("malloc", addr, len);

    // Return the address of the new pointer
    return ptr;
}


/**
 * A function hook to robustly allocate heap memory with realloc.
 * @param s - Swimmer responsible for allocation.
 * @param addr - Address that called the original function.
 */
triton::uint64 koi_realloc(Swimmer *s, triton::uint64 addr) {
    // Get and validate the length of allocation
    triton::uint64 new_len = __getSatisfiableRegisterValue(s, s->registers.x86_rsi);
    if(new_len == 0) return 0;

    // Get the pointer to be reallocated
    triton::uint64 old_ptr = __getSatisfiableRegisterValue(s, s->registers.x86_rdi);
    
    // When the pointer is null, the behavior is the same as calling malloc
    if(old_ptr == 0) {
        old_ptr = s->allocateHeapMemory("realloc", addr, new_len);
        return old_ptr;
    }

    // If old_ptr is not null, it must have been previously allocated
    if(!s->statHeapMemory(old_ptr, true))
        return 0;

    // Get the current size of the pointer
    triton::uint64 old_len = s->getAllocatedLength(old_ptr);

    // Allocate and symbolize new chunk of memory for the new size
    triton::uint64 new_ptr = s->allocateHeapMemory("realloc", addr, new_len);

    // Iterate the old memory to copy concretes and constraints to the new pointer
    __copyConcretesAndConstraints(s, new_ptr, old_ptr, std::min(old_len, new_len));

    // Free the old memory
    s->freeHeapMemory(old_ptr, addr);

    // Return the address of the new pointer
    return new_ptr;
}
