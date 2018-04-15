#include <ymir/semantic/pack/DestructSolver.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/semantic/value/BoolValue.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/ast/_.hh>


namespace semantic {

    using namespace syntax;
    
    std::string DestructSolution::toString () {
	return Ymir::OutBuffer (score, " ", valid, " ", caster, " ", created, " -> ", test).str ();
    }

    DestructSolver DestructSolver::__instance__;
    
    DestructSolver & DestructSolver::instance () {
	return __instance__;
    }

    bool DestructSolver::merge (DestructSolution & left, const DestructSolution & right, const std::string &op) {
	if (left.test) {
	    Word tok {left.test-> token, op};
	    left.test = (new (Z0) IBinary (tok, left.test, right.test))-> expression ();
	} else left.test = right.test;
	
	if (!left.test) return false;
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

	if (op == Token::DPIPE)
	    left.immutable = left.immutable || right.immutable;
	else
	    left.immutable = left.immutable && right.immutable;

	if (left.test-> info-> value ()) {
	    left.immutable = left.test-> info-> value ()-> to <IBoolValue> ()-> isTrue ();
	}
	
	return true;
    }
    
    DestructSolution DestructSolver::solve (Expression left, Expression right) {
	if (auto mpair = left-> to <IMatchPair> ()) {
	    return solvePair (mpair, right);
	} else if (auto bin = left-> to <IBinary> ()) {
	    return solveBinary (bin, right);
	} else if (auto var = left-> to <IVar> ()) {
	    return solveVar (var, right);
	} else if (left-> is <IIgnore> ()) {
	    return solveIgnore (right);
	} else if (auto tu = left-> to <IConstTuple> ()) {
	    return solveTuple (tu, right);
	} else if (auto st = left-> to <IStructCst> ()) {
	    return solveStructCst (st, right);
	} 
	return solveNormal (left, right);
    }

    DestructSolution DestructSolver::solveStructCst (StructCst left, Expression right) {
	InfoType type = right-> info-> type;
	if (auto ref = type-> to <IRefInfo> ()) type = ref-> content ();	
	if (auto str = type-> to <IStructInfo> ()) {
	    DestructSolution soluce {0, true};
	    auto type = left-> getLeft ()-> expression ();
	    if (type == NULL) return DestructSolution (0, false);
	    if (!type-> isType ()) {
		Ymir::Error::useAsType (type-> token);
		return DestructSolution (0, false);
	    }
	    
	    auto strCst = type-> info-> type-> to <IStructCstInfo> ();
	    if (!strCst) return DestructSolution (0, false);	    
	    if (left-> getExprs ().size () != str-> getTypes ().size ()) {
	    	return DestructSolution {0, false};
	    }

	    for (auto it : Ymir::r (0, str-> getTypes ().size ())) {
		auto exp = new (Z0) IExpand (right-> token, right, it);		
		exp-> info = new (Z0) ISymbol (exp-> token, str-> getTypes  ()[it]-> clone ());
		exp-> info-> isConst (right-> info-> isConst ());
		
		auto res = solve (left-> getExprs () [it], exp);
		if (!res.valid || !merge (soluce, res, Token::AND))
		    return DestructSolution {0, false};
	    }
	    return soluce;
	    
	}
	return DestructSolution (0, false);
    }
    
    DestructSolution DestructSolver::solveTuple (ConstTuple left, Expression right) {
	InfoType type = right-> info-> type;
	if (auto ref = type-> to <IRefInfo> ()) type = ref-> content ();	
	if (auto tuple = type-> to <ITupleInfo> ()) {
	    DestructSolution soluce {0, true};
	    if (left-> getExprs ().size () != tuple-> nbParams ()) {
		return DestructSolution {0, false};
	    }

	    for (auto it : Ymir::r (0, tuple-> nbParams ())) {
		auto exp = new (Z0) IExpand (right-> token, right, it);
		exp-> info = new (Z0) ISymbol (exp-> token, tuple-> getParams ()[it]-> clone ());
		exp-> info-> isConst (right-> info-> isConst ()); 
		auto res = solve (left-> getExprs () [it], exp);
		if (!res.valid || !merge (soluce, res, Token::AND))
		    return DestructSolution {0, false};
	    }
	    return soluce;
	} else return DestructSolution {0, false};
    }
    
    DestructSolution DestructSolver::solveIgnore (Expression right) {
	DestructSolution soluce {__VAR__, true};
	auto test = new (Z0) IBool (right-> token);
	test-> getValue () = true;	
	soluce.test = test-> expression ();
	soluce.immutable = true;
	return soluce;
    }

    DestructSolution DestructSolver::solveVar (Var var, Expression right, bool canNormal) {
	if (auto typed = var-> to <ITypedVar> ()) return solveTyped (typed, right);
    	if (Table::instance ().get (var-> token.getStr ()) != NULL && canNormal)
    	    return solveNormal (var, right);

    	Expression bin = NULL;
	auto aux = new (Z0) IVar (var-> token);
    	if (var-> deco == Keys::REF) {
	    if (right-> info-> type-> is <IRefInfo> ()) {
		aux-> info = new (Z0) ISymbol (var-> token, right-> info-> type-> clone ());
		Table::instance ().insert (aux-> info);
	    
		bin = (new (Z0) IAffectGeneric ({var-> token, Token::EQUAL},
						aux,
						right, false))-> expression ();
	    } else {
		aux-> info = new (Z0) ISymbol (var-> token, new (Z0) IRefInfo (false, right-> info-> type-> clone ()));
		Table::instance ().insert (aux-> info);
		
		bin = (new (Z0) IAffectGeneric ({var-> token, Token::EQUAL},
						aux,
						right, true))-> expression ();
	    }
    	} else {
    	    aux-> info = new (Z0) ISymbol (var-> token, new (Z0) IUndefInfo ());
    	    aux-> info-> isConst (false);
    	    Table::instance ().insert (aux-> info);

    	    Word affTok {var-> token, Token::EQUAL};
	    bin = (new (Z0) IBinary (affTok, aux, right))-> expression ();
    	    if (!bin) return DestructSolution (0, false);
    }
    
    	DestructSolution soluce {__VAR__, true};
    	soluce.created.push_back (aux);
    	soluce.caster.push_back (bin);
    	auto test = new (Z0) IBool (var-> token);
    	test-> getValue () = true;
    	soluce.test = test-> expression ();
    	soluce.immutable = true;
	
    	return soluce;	
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
	if (bin-> info-> value ()) {
	    soluce.immutable = bin-> info-> value ()-> to <IBoolValue> ()-> isTrue ();
	}
	return soluce;
    }
        
    DestructSolution DestructSolver::solveNormal (Expression left, Expression right) {
	auto value = left-> expression ();
	if (value == NULL) return DestructSolution (0, false);

	if (!value-> info-> type-> CompOp (right-> info-> type))
	    return DestructSolution (0, false);

	Word tok {left-> token, Token::DEQUAL};
	if (right-> info-> type-> is <IPtrInfo> ()) {
	    tok.setStr (Keys::IS);
	}
	
	Expression bin = new (Z0) IBinary (tok, right, left);
	bin = bin-> expression ();
	if (!bin) return DestructSolution (0, false);
	DestructSolution soluce {__VALUE__, true};
	soluce.test = bin;
	if (bin-> info-> value ()) {
	    soluce.immutable = bin-> info-> value ()-> to <IBoolValue> ()-> isTrue ();
	}
	return soluce;
    }

    DestructSolution DestructSolver::solveTyped (TypedVar left, Expression right) {
	auto ltype = left-> getType ();
	auto rtype = right-> info-> type;
	if (auto ref = rtype-> to <IRefInfo> ())
	    rtype = ref-> content ();

	if (auto ref = ltype-> to <IRefInfo> ())
	    ltype = ref-> content ();
	
	if (ltype-> isSame (rtype)) {
	    auto var = new (Z0) IVar (left-> token);
	    var-> deco = left-> deco;
	    return solveVar (var, right, false);
	} else return DestructSolution (0, false);
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
