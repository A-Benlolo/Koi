#include <triton/context.hpp>
#include "Koi/stackframe.h"


/********************/
/* PUBLIC FUNCTIONS */
/********************/

/**
 * Default constructor.
 * @return a new Stackframe.
 */
Stackframe::Stackframe() : Cove() {
    accesses = {};
}


/**
 * Alternate constructor
 * @param a - Address of the stackframe
 * @param s - Size of the stackframe
 * @return a new Stackframe.
 */
Stackframe::Stackframe(triton::uint64 a, size_t s) : Cove(a, s) {
    accesses = {0, s};
}


/**
 * Add an access offset to a Stackframe
 * @param offs - Offset of the access.
 * @return true if the access is new
 */
bool Stackframe::addAccess(triton::uint64 offs) {
    auto it = std::find(accesses.begin(), accesses.end(), offs);
    if(it == accesses.end()) {
        accesses.push_back(offs);
        std::sort(accesses.begin(), accesses.end());
        return true;
    }
    return false;
}


/**
 * Get the length until the next access or end of stack.
 * @param offs - Offset to start search from.
 * @return the length until the next access.
 */
size_t Stackframe::getAccessGap(triton::uint64 offs) {
    // Finds a gap until next access
    for(size_t i = 1; i < accesses.size(); i++) {
        if(offs >= accesses[i])
            return offs - accesses[i-1];
    }
    return 0;
}


/**
 * Update the stackframe information
 * @param a - New address of the stackframe
 * @param s - New size of the stackframe
 */
void Stackframe::update(triton::uint64 a, size_t s) {
    addr = a;
    sz = s;
    accesses = {0, s};
}


/**
 * Expand a stackframe by a set number of bytes
 * @param b - Number of bytes to extend by.
 */
 void Stackframe::extend(size_t b) {
    sz += b;
    addAccess(triton::uint64(sz));
}


/**
 * Shrink a stackframe by a set number of bytes
 * @param b - Number of bytes to shrink by.
 */
 void Stackframe::shrink(size_t b) {
    sz -= b;
}