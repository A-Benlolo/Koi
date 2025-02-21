#ifndef KOIBAITSTRING_H
#define KOIBAITSTRING_H


#include <triton/context.hpp>
#include "Koi/swimmer.h"


/**
 * A function hook to robustly perform strchr
 * @param s - Swimmer responsible for the search.
 * @param addr - Address that called the original function.
 * @return a pointer to the first requested character in the string.
 */
triton::uint64 koi_strchr(Swimmer *s, triton::uint64 addr);


/**
 * A function hook to robustly perform a string copy.
 * @param s - Swimmer responsible for the copy.
 * @param addr - Address that called the original function.
 * @return a copy of the destination
 */
triton::uint64 koi_strcpy(Swimmer *s, triton::uint64 addr);


/**
 * A function hook to robustly perform a string length calculation.
 * @param s - Swimmer responsible for the calculation.
 * @param addr - Address that called the original function.
 * @return the deduced length of the source string.
 */
triton::uint64 koi_strlen(Swimmer *s, triton::uint64 addr);


/**
 * A function hook to robustly perform a limited string copy.
 * @param s - Swimmer responsible for the copy.
 * @param addr - Address that called the original function.
 */
triton::uint64 koi_strncpy(Swimmer *s, triton::uint64 addr);


#endif