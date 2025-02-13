#include <iomanip>
#include <triton/context.hpp>
#include "Koi/buffer.h"


/********************/
/* PUBLIC FUNCTIONS */
/********************/


/**
 * Alternate constructor
 * @param id - Identifying name created the buffer (i.e. malloc, calloc, etc)
 * @param addr - Address that the buffer was created.
 * @param ptr - Address of the buffer
 * @param len - Size of the buffer
 * @return a new Buffer with no symbolic variables.
 */
Buffer::Buffer(std::string id, triton::uint64 addr, triton::uint64 ptr, size_t len) : Cove(ptr, len) {
    std::stringstream ss;
    ss << id << "<--0x" << std::hex << addr;
    alias = ss.str();
    vars = {};
    state = BufferState::Live;
    sink = addr;
    origin = addr;
}


/**
 * Mark a buffer as dead
 * @param addr - Point at which the buffer is killed.
 * @return true if buffer was not previously dead.
 */
bool Buffer::kill(triton::uint64 addr) {
    if(state == BufferState::Dead)
        return false;
    sink = addr;
    state = BufferState::Dead;
    return true;
}


/**
 * Check the liveliness of a Buffer.
 * @return true if the buffer is live, else false.
 */
bool Buffer::stat() {
    return state == BufferState::Live;
}


/**
 * Get the last address where the buffer state was changed
 * @return the buffer's sink.
 */
triton::uint64 Buffer::getSink() {
    return sink;
}


/**
 * Get the address where the buffer was originally created.
 * @return the buffer's origin.
 */
triton::uint64 Buffer::getOrigin() {
    return origin;
}