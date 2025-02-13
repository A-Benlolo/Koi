#ifndef BUFFER_H
#define BUFFER_H

#include "Koi/cove.h"
#include "Koi/buffer.h"


class Buffer: public Cove {
protected:
    /* A buffer can be live or dead (allocated or free'd) */
    enum BufferState {
        Live,
        Dead,
    };
    BufferState state;
    triton::uint64 origin;
    triton::uint64 sink;

public:
    /* A buffer is a Cove with an alias, and symbolic variables */
    std::string alias;
    std::vector<triton::engines::symbolic::SharedSymbolicVariable> vars;


    /**
     * Alternate constructor
     * @param id - Identifying name created the buffer (i.e. malloc, calloc, etc)
     * @param addr - Address that the buffer was created.
     * @param ptr - Address of the buffer
     * @param len - Size of the buffer
     * @return a new Buffer.
     */
    Buffer(std::string id, triton::uint64 addr, triton::uint64 ptr, size_t len);


    /**
     * Mark a buffer as dead.
     * @param addr - Point at which the buffer is killed.
     * @return true if buffer was not previously dead.
     */
    bool kill(triton::uint64 addr);


    /**
     * Check the liveliness of a Buffer.
     * @return true if the buffer is live, else false.
     */
    bool stat();


    /**
     * Get the last address where the buffer state was changed
     * @return the buffer's sink.
     */
    triton::uint64 getSink();


    /**
     * Get the address where the buffer was originally created.
     * @return the buffer's origin.
     */
     triton::uint64 getOrigin();
};


#endif