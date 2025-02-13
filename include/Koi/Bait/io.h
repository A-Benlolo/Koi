#ifndef KOIBAITIO_H
#define KOIBAITIO_H


#include <triton/context.hpp>
#include "Koi/swimmer.h"


/**
 * A function hook to robustly perform a read from a file.
 * @param s - Swimmer responsible for the read.
 * @param addr - Address that called the original function.
 * @return the number of read bytes.
 */
triton::uint64 koi_fgets(Swimmer *s, triton::uint64 addr);

#endif