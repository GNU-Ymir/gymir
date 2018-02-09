#include <ymir/semantic/pack/DestructSolver.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/ast/_.hh>


namespace semantic {

    using namespace syntax;
    
    std::string DestructSolution::toString () {
	return Ymir::OutBuffer (score, " ", valid, " ", caster, " ", created).str ();
    }

    DestructSolver DestructSolver::__instance__;
    
    DestructSolver & DestructSolver::instance () {
	return __instance__;
    }

    bool DestructSolver::merge (DestructSolution & left, const DestructSolution & right, const std::string &op) {
	Word tok {left.test-> token, op};
	left.test = (new (Z0) IBinary (tok, left.test, right.test))-> expression ();
	for (auto it : right.caster)
	    left.caster.push_back (it);

	for (auto it : right.created) {
	    for (auto it_ : left.created) {
		if (it-> token.getStr () == it_-> token.getStr ()) {
		    Ymir::Error::shadowingVar (it-> token, it_-> token);
		    return false;
		}
	    }
	    left.created.push_back (it);	    
	}
	return true;
    }
    
    DestructSolution DestructSolver::solve (Expression left, Expression right) {
	if (auto mpair = left-> to <IMatchPair> ()) {
	    return solvePair (mpair, right);
	} else if (auto bin = left-> to <IBinary> ()) {
	    return solveBinary (bin, right);
	}
	return solveNormal (left, right);
    }

    DestructSolution DestructSolver::solvePair (MatchPair pair, Expression right) {
	auto lvalue = pair-> getLeft ()-> expression ();
	auto rvalue = pair-> getRight ()-> expression ();

	if (lvalue == NULL || rvalue == NULL) return DestructSolution (0, false);

	if (!lvalue-> info-> type-> CompOp (rvalue-> info-> type)) {
	    Ymir::Error::incompatibleTypes (pair-> token, lvalue-> info, rvalue-> info-> type);
	    return DestructSolution (0, false);
	}

	if (!right-> info-> type-> CompOp (lvalue-> info-> type))
	    return DestructSolution (0, false);

	Word tokLeft {pair-> token, Token::SUP_EQUAL};
	Expression binLeft = (new (Z0) IBinary (tokLeft, right, lvalue))-> expression ();
	if (binLeft == NULL) return DestructSolution (0, false);

	Word tokRight {pair-> token, Token::INF_EQUAL};
	Expression binRight = (new (Z0) IBinary (tokRight, right, rvalue))-> expression ();
	if (binRight == NULL) return DestructSolution (0, false);

	Word dand {pair-> token, Token::DAND};
	Expression bin = (new (Z0) IBinary (dand, binLeft, binRight))-> expression ();	
	if (!bin) return DestructSolution (0, false);
	DestructSolution soluce {__VALUE__, true};
	soluce.test = bin;
	return soluce;
    }
    
    DestructSolution DestructSolver::solveNormal (Expression left, Expression right) {
	auto value = left-> expression ();
	if (value == NULL) return DestructSolution (0, false);

	if (!right-> info-> type-> CompOp (value-> info-> type))
	    return DestructSolution (0, false);

	Word tok {left-> token, Token::DEQUAL};
	Expression bin = new (Z0) IBinary (tok, left, right);
	bin = bin-> expression ();
	if (!bin) return DestructSolution (0, false);
	DestructSolution soluce {__VALUE__, true};
	soluce.test = bin;
	
	return soluce;
    }

    DestructSolution DestructSolver::solveBinary (Binary left, Expression right) {
	if (left-> token != Token::PIPE) return solveNormal (left, right);

	auto soluceLeft = solve (left-> getLeft (), right);
	if (!soluceLeft.valid) return DestructSolution (0, false);
	auto soluceRight = solve (left-> getRight (), right);
	if (!soluceRight.valid || !merge (soluceLeft, soluceRight, Token::DPIPE))
	    return DestructSolution (0, false);
	return soluceLeft;
    }


    
}
