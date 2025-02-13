#include <triton/context.hpp>
#include "Koi/cove.h"


/********************/
/* PUBLIC FUNCTIONS */
/********************/

/**
 * Default constructor.
 * @return a new Cove.
 */
Cove::Cove() {
    addr = 0;
    sz = 0;
}


/**
 * Alternate constructor
 * @param a - Address of the cove
 * @param s - Size of the cove
 * @return a new Cove.
 */
Cove::Cove(triton::uint64 a, size_t s) {
    addr = a;
    sz = s;
}

/**
 * Get the cove start address.
 * @return the cove start address.
 */
triton::uint64 Cove::getAddress() {
    return addr;
}


/**
 * Get the cove size.
 * @return the cove size.
 */
triton::uint64 Cove::getSize() {
    return sz;
}