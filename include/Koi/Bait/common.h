#ifndef KOIBAITCOMMON_H
#define KOIBAITCOMMON_H

#include <triton/context.hpp>
#include "Koi/swimmer.h"

/**
 * Deduce a satisfiable register value.
 * @param s - Swimmer the register belongs to.
 * @param reg - Register to deduce.
 * @param err - Value that would indicate failure (default=0)
 * @return a concrete value that satisfies any existing constraints (or err).
 */
triton::uint64 __getSatisfiableRegisterValue(Swimmer *s, triton::arch::Register &reg, triton::uint64 err=0);


/**
 * Copy concrete values and constraints from a source to destination.
 * @param s - Swimmer that the addresses belong to.
 * @param dst - Address to copy to.
 * @param src - Address to copy from.
 * @param len - Number of bytes to copy.
 */
void __copyConcretesAndConstraints(Swimmer *s, triton::uint64 dst, triton::uint64 src, size_t len);

#endif