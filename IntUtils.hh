#pragma once

#include "Expression.hh"
#include "Tree.hh"
#include <iostream>

namespace Semantic {

    struct IntUtils {

	static Ymir::Tree InstAffInt (location_t locus, Syntax::ExpressionPtr lexp , Syntax::ExpressionPtr rexp ) {
	    auto left = lexp-> treeExpr ();
	    auto right = rexp-> treeExpr ();

	    return Ymir::buildTree (
		MODIFY_EXPR, locus, void_type_node, left, right
	    );	    
	}

	static Ymir::Tree InstAffChar (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}


	static Ymir::Tree InstAffFloat (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstPlusAffInt (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstPlusAffChar (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstPlusAffFloat (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstMulAffInt (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstMulAffChar (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstMulAffFloat (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstSubAffInt (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstSubAffChar (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstSubAffFloat (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstDivAffInt (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstDivAffChar (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstDivAffFloat (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstInfInt (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstInfChar (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	    
	}

	static Ymir::Tree InstInfFloat (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstSupInt (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstSupChar (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstSupFloat (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstEqualsInt (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstEqualsChar (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstEqualsFloat (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}
	
	static Ymir::Tree InstInfEqualsInt (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstInfEqualsChar (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstInfEqualsFloat (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstSupEqualsInt (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstSupEqualsChar (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}
	
	static Ymir::Tree InstSupEqualsFloat (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstNotEqualsInt (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstNotEqualsChar (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstNotEqualsFloat (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}	

	static Ymir::Tree InstPlusInt (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstSubInt (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstOrBitInt (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstLeftBitInt (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstRightBitInt (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstXorInt (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstMulInt (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstDivInt (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstAndBitInt (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}

	static Ymir::Tree InstModuloInt (location_t , Syntax::ExpressionPtr , Syntax::ExpressionPtr ) {
	    return Ymir::Tree ();
	}       	

    };
    
}
