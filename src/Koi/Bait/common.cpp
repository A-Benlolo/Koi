#include <algorithm>
#include <triton/context.hpp>
#include "Koi/Bait/common.h"


/**
 * Deduce a satisfiable register value.
 * @param s - Swimmer the register belongs to.
 * @param reg - Register to deduce.
 * @param err - Value that would indicate failure (default=0)
 * @return a concrete value that satisfies any existing constraints (or err).
 */
triton::uint64 __getSatisfiableRegisterValue(Swimmer *s, triton::arch::Register &reg, triton::uint64 err) {
    // Concrete register value
    if(!s->isRegisterSymbolized(reg))
        return triton::uint64(s->getConcreteRegisterValue(reg));

    // Symbolic register constraints 
    const triton::engines::symbolic::SharedSymbolicExpression &expr = s->getSymbolicRegister(reg);
    const triton::ast::SharedAbstractNode &ast = expr->getAst();

    // Solve for the constraints
    if(ast->isLogical()) {
        auto model = s->getModel(ast);

        // Return a solution or err if not satisfiable
        if(model.size() > 0)
            return triton::uint64(model.begin()->second.getValue());
    }
    return err;
}


/**
 * Copy concrete values and constraints from a source to destination.
 * @param s - Swimmer that the addresses belong to.
 * @param dst - Address to copy to.
 * @param src - Address to copy from.
 * @param len - Number of bytes to copy.
 */
void __copyConcretesAndConstraints(Swimmer *s, triton::uint64 dst, triton::uint64 src, size_t len) {
    for(size_t i = 0; i < len; i++) {
        // Symbolic memory
        if(s->isMemorySymbolized(src + i)) {
            auto ast = s->getSymbolicMemory(src + i)->getAst();
            s->getSymbolicMemory(dst + i)->setAst(ast);
        }

        // Concrete memory
        else if(s->isConcreteMemoryValueDefined(src + i)) {
            triton::uint8 b = s->getConcreteMemoryValue(src + i);
            s->setConcreteMemoryValue(dst + i, b);
        }

        // Undefined memory 
        else {
            s->setConcreteMemoryValue(dst + i, 0);
        }
    }
}
