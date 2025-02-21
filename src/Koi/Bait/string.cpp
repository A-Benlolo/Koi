#include <triton/context.hpp>
#include "Koi/swimmer.h"
#include "Koi/Bait/common.h"
#include "Koi/Bait/string.h"


triton::uint64 __deduceStringSize(Swimmer *s, triton::uint64 ptr);


/**
 * A function hook to robustly perform strchr
 * @param s - Swimmer responsible for the search.
 * @param addr - Address that called the original function.
 * @return a pointer to the first occurance of the requested character or null.
 */
 triton::uint64 koi_strchr(Swimmer *s, triton::uint64 addr) {
    // Assume the pointer is concrete
    triton::uint64 ptr_in = __getSatisfiableRegisterValue(s, s->registers.x86_rdi);

    // Concrete string
    if(!s->isMemorySymbolized(ptr_in)) {
        // Concrete character
        if(!s->isRegisterSymbolized(s->registers.x86_rsi)) {
            triton::uint64 chr = triton::uint64(s->getConcreteRegisterValue(s->registers.x86_rsi));
            triton::uint8 byte;
            do {
                byte = s->getConcreteMemoryValue(ptr_in);
                if(byte == chr)
                    return ptr_in;
                ptr_in++;
            } while(byte != 0);
        }

        // Symbolic character
        else {
            auto ast = s->getSymbolicRegister(s->registers.x86_rsi)->getAst();
            auto astCtxt = s->getAstContext();
            triton::uint8 byte;
            do {
                byte = s->getConcreteMemoryValue(ptr_in);
                auto model = s->getModel(astCtxt->equal(ast, astCtxt->bv(byte, 8)));
                if(model.size() > 0)
                    return triton::uint64(model.begin()->second.getValue());
                ptr_in++;
            } while(byte != 0);
        }
    }

    // Symbolic string
    else {
        // Use strlen to determine length of source
        triton::uint64 full_len = koi_strlen(s, 0);
        auto astCtxt = s->getAstContext();

        // Concrete character, find earliest match
        if(!s->isRegisterSymbolized(s->registers.x86_rsi)) {
            triton::uint64 chr = triton::uint64(s->getConcreteRegisterValue(s->registers.x86_rsi));
            for(triton::uint64 i = 0; i < full_len; i++) {
                auto ast = s->getSymbolicMemory(ptr_in + i)->getAst();
                auto isMatch = astCtxt->equal(ast, astCtxt->bv(chr, 8));
                if(s->getModel(isMatch).size() > 0)
                    return ptr_in + i;
            }
        }

        // Symbolic character, find earliest match
        else {
            auto chrAst = s->getSymbolicRegister(s->registers.x86_rsi)->getAst();
            for(triton::uint64 i = 0; i < full_len; i++) {
                auto strAst = s->getSymbolicMemory(ptr_in + i)->getAst();
                auto isMatch = astCtxt->equal(chrAst, strAst);
                if(s->getModel(isMatch).size() > 0)
                    return ptr_in + i;
            }
        }
    }
    
    // No answer fouund, return null
    return 0;
 }


/**
 * A function hook to robustly perform a string copy.
 * @param s - Swimmer responsible for the copy.
 * @param addr - Address that called the original function.
 * @return a copy of the destination
 */
triton::uint64 koi_strcpy(Swimmer *s, triton::uint64 addr) {
    // Get the source and destination pointers
    triton::uint64 dptr = __getSatisfiableRegisterValue(s, s->registers.x86_rdi);
    triton::uint64 sptr = __getSatisfiableRegisterValue(s, s->registers.x86_rsi);

    // Use strlen to determine length of source
    s->setConcreteRegisterValue(s->registers.x86_rdi, sptr);
    triton::uint64 slen = koi_strlen(s, 0);
    s->setConcreteRegisterValue(s->registers.x86_rdi, dptr);

    // Perform the copy, cutting off after slen bytes if there's an overflow
    __copyConcretesAndConstraints(s, dptr, sptr, std::min(koi_strlen(s, dptr), slen));

    // A copy of the desitnation is returned
    return dptr;
}


/**
 * A function hook to robustly perform a string length calculation.
 * @param s - Swimmer responsible for the calculation.
 * @param addr - Address that called the original function.
 * @return the deduced length of the source string.
 */
triton::uint64 koi_strlen(Swimmer *s, triton::uint64 addr) {
    // Get and validate the string pointer
    triton::uint64 ptr = __getSatisfiableRegisterValue(s, s->registers.x86_rdi);
    if(ptr == 0) return 0;

    // Trivial case: a concrete string is already defined
    triton::uint64 len = s->readString(ptr).length();
    if(len > 0) return len;

    // Heap string: get the allocation length (includes null)
    triton::uint64 full_len = s->getAllocatedLength(ptr);

    // Stack string: get the best-guess length (includes null)
    if(full_len == 0)
        full_len = s->getStackBufferLength(ptr);

    // No string found
    if(full_len == 0)
        return 0;

    // Search for earliest defined or latest satisfiable null byte
    len = full_len;
    auto astCtxt = s->getAstContext();
    triton::uint64 symbolic_null = 0;
    for(triton::uint64 i = full_len - 1; ; i--) {
        // Check for symbolic null
        if(symbolic_null == 0 && s->isMemorySymbolized(ptr + i)) {
            auto ast = s->getSymbolicMemory(ptr + i)->getAst();
            auto isNull = astCtxt->equal(ast, astCtxt->bv(0, 8));
            if(s->getModel(isNull).size() > 0)
                symbolic_null = i;
        }

        // Check for concrete null
        else if(s->isConcreteMemoryValueDefined(ptr + i)) {
            if(s->getConcreteMemoryValue(ptr + i) == 0)
                len = i;
        }

        // Nasty break because unsigned will never go below zero
        if(i == 0) break;
    }

    // If no concrete null bytes were found, assume longest satisfiable string
    if(len == full_len) {
        len = symbolic_null;
    }

    // Return the length of the string
    return len;
}


/**
 * A function hook to robustly perform a limited string copy.
 * @param s - Swimmer responsible for the copy.
 * @param addr - Address that called the original function.
 */
triton::uint64 koi_strncpy(Swimmer *s, triton::uint64 addr) {
    // Get the source and destination pointers and the length
    triton::uint64 dptr = __getSatisfiableRegisterValue(s, s->registers.x86_rdi);
    triton::uint64 sptr = __getSatisfiableRegisterValue(s, s->registers.x86_rsi);
    triton::uint64 n = __getSatisfiableRegisterValue(s, s->registers.x86_rdx);

    // Use strlen to determine length of source
    s->setConcreteRegisterValue(s->registers.x86_rdi, sptr);
    triton::uint64 slen = koi_strlen(s, sptr);
    s->setConcreteRegisterValue(s->registers.x86_rdi, dptr);

    // Perform the copy
    __copyConcretesAndConstraints(s, dptr, sptr, std::min(n, slen));

    // A copy of the desitnation is returned
    return dptr;
}


/********************/
/* HELPER FUNCTIONS */
/********************/

triton::uint64 __deduceStringSize(Swimmer *s, triton::uint64 ptr) {
    size_t len = s->readString(ptr).length() + 1;
    if(len == 1) {
        len = s->getAllocatedLength(ptr);
        if(len == 0) {
            len = s->getStackBufferLength(ptr);
        }
    }
    return len;
}