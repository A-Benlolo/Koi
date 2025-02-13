#include <triton/context.hpp>
#include "Koi/swimmer.h"

/**
 * A function hook to robustly perform a read from a file.
 * @param s - Swimmer responsible for the read.
 * @param addr - Address that called the original function.
 * @return the number of read bytes.
 */
triton::uint64 koi_fgets(Swimmer *s, triton::uint64 addr) {
    triton::uint64 ptr = triton::uint64(s->getConcreteRegisterValue(s->registers.x86_rdi));
    triton::uint64 n = triton::uint64(s->getConcreteRegisterValue(s->registers.x86_rsi));
    s->symbolizeNamedMemory("fgets", ptr, addr, n);
    s->setConcreteMemoryValue(ptr+n-1, 0);
    return n;
}