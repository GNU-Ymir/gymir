#include <ymir/semantic/pack/DestructSolver.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {

    using namespace syntax;
    
    std::string DestructSolution::toString () {
	return Ymir::OutBuffer (score, " ", valid, " ", caster, " ", created).str ();
    }

    DestructSolver DestructSolver::__instance__;
    
    DestructSolver & DestructSolver::instance () {
	return __instance__;
    }

    DestructSolution DestructSolver::solve (Expression left, Expression right) {
	println ("ici");
	return DestructSolution (0, false);
    }
        
}
