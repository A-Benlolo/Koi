#ifndef STACKFRAME_H
#define STACKFRAME_H

#include "Koi/cove.h"
#include "Koi/stackframe.h"

class Stackframe: public Cove {
private:
    /* A stackframe is a Cove with a record of accessed offsets */
    std::vector<triton::uint64> accesses;

public:
    /**
     * Default constructor.
     * @return a new Stackframe.
     */
    Stackframe();


    /**
     * Alternate constructor
     * @param a - Address of the stackframe
     * @param s - Size of the stackframe
     * @return a new Stackframe.
     */
    Stackframe(triton::uint64 a, size_t s);


    /**
     * Add an access offset to a Stackframe
     * @param offs - Offset of the access.
     * @return true if the access is new
     */
    bool addAccess(triton::uint64 offs);


    /**
     * Get the length until the next access.
     * @param offs - Offset to start search from.
     * @return the length until the next access.
     */
    size_t getAccessGap(triton::uint64 offs);


    /**
     * Update the stackframe information
     * @param a - New address of the stackframe
     * @param s - New size of the stackframe
     */
    void update(triton::uint64 a, size_t s);


    /**
     * Expand a stackframe by a set number of bytes
     * @param b - Number of bytes to extend by.
     */
    void extend(size_t b);


    /**
     * Shrink a stackframe by a set number of bytes
     * @param b - Number of bytes to shrink by.
     */
     void shrink(size_t b);
};


#endif