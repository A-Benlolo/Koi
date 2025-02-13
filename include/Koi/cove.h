#ifndef COVE_H
#define COVE_H


class Cove {
protected:
    /* A cove has an address and size */
    triton::uint64 addr;
    size_t sz;

public:
    /**
     * Default constructor.
     * @return a new Cove.
     */
    Cove();


    /**
     * Alternate constructor.
     * @param a - Address of the cove.
     * @param s - Size of the cove.
     * @return a new Cove.
     */
    Cove(triton::uint64 a, size_t s);


    /**
     * Get the Cove start address.
     * @return the Cove start address.
     */
    triton::uint64 getAddress();


    /**
     * Get the Cove size.
     * @return the Cove size.
     */
    triton::uint64 getSize();
};

#endif