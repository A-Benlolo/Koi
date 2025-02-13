#ifndef KOIBAITALLOC_H
#define KOIBAITALLOC_H


#include <triton/context.hpp>
#include "Koi/swimmer.h"


/**
 * A function hook to robustly allocate heap memory with calloc.
 * @param s - Swimmer responsible for allocation.
 * @param addr - Address that called the original function.
 */
triton::uint64 koi_calloc(Swimmer *s, triton::uint64 addr);


/**
 * A function hook to robustly free heap memory.
 * @param s - Swimmer responsible for freeing.
 * @param addr - Address that called the original function.
 */
triton::uint64 koi_free(Swimmer *s, triton::uint64 addr);


/**
 * A function hook to robustly allocate heap memory with malloc.
 * @param s - Swimmer responsible for allocation.
 * @param addr - Address that called the original function.
 */
triton::uint64 koi_malloc(Swimmer *s, triton::uint64 addr);


/**
 * A function hook to robustly allocate heap memory with realloc.
 * @param s - Swimmer responsible for allocation.
 * @param addr - Address that called the original function.
 */
triton::uint64 koi_realloc(Swimmer *s, triton::uint64 addr);


#endif