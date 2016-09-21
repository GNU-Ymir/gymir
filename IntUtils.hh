#pragma once

#include "Expression.hh"
#include "Tree.hh"

namespace Semantic {

    struct IntUtils {

	static Ymir::Tree InstAffInt (Syntax::ExpressionPtr left, Syntax::ExpressionPtr right) {
	    return Ymir::Tree ();   
	}

	static Ymir::Tree InstAffChar (Syntax::ExpressionPtr left, Syntax::ExpressionPtr right) {
	    return Ymir::Tree ();
	}


	static Ymir::Tree InstAffFloat (Syntax::ExpressionPtr left, Syntax::ExpressionPtr right) {
	    return Ymir::Tree ();
	}

    };
    
}
