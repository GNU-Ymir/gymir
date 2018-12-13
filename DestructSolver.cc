#include <ymir/semantic/pack/DestructSolver.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/semantic/value/BoolValue.hh>
#include <ymir/semantic/object/AggregateInfo.hh>
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
	if (auto bin = left-> to <IBinary> ()) {
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
	InfoType type = right-> info-> type ();
	while (auto ref = type-> to <IRefInfo> ()) type = ref-> content ();	
	if (auto str = type-> to <IStructInfo> ()) {
	    DestructSolution soluce {0, true};
	    if (!left-> getLeft ()-> is<IIgnore> ()) {
		auto type = left-> getLeft ()-> toType ();
		if (!str-> isSame (type-> info-> type ())) return DestructSolution (0, false);		
	    }

	    std::set <int> toSkip;
	    uint current = 0, it = 0;
	    while (current < left-> getExprs ().size () && it < str-> getTypes ().size ()) {
		if (auto named = left-> getExprs () [current]-> to <INamedExpression> ()) {
		    auto itnamed = str-> indexOf (named-> token.getStr ());
		    if (itnamed == -1) {
			// The type is the same as the structure, but it does not have any attr 'named-> token'
			if (!left-> getLeft ()-> is <IIgnore> ())
			    Ymir::Error::undefAttr (named-> token, right-> info, new (Z0) IVar (named-> token));
			return DestructSolution (0, false);
		    }
		    
		    toSkip.insert (itnamed);
		    auto exp = new (Z0) IExpand (right-> token, right, itnamed);		
		    exp-> info = new (Z0) ISymbol (exp-> token, exp, str-> getTypes  ()[itnamed]-> clone ());
		    exp-> info-> isConst (right-> info-> isConst ());
		    
		    auto res = solve (named-> getValue (), exp);
		    if (!res.valid || !merge (soluce, res, Token::AND))
			return DestructSolution {0, false};
		    
		} else {
		    if (toSkip.find (it) != toSkip.end ()) {
			it += 1;
			continue;
		    }

		    auto exp = new (Z0) IExpand (right-> token, right, it);		
		    exp-> info = new (Z0) ISymbol (exp-> token, exp, str-> getTypes  ()[it]-> clone ());
		    exp-> info-> isConst (right-> info-> isConst ());
		    
		    auto res = solve (left-> getExprs () [current], exp);
		    if (!res.valid || !merge (soluce, res, Token::AND))
			return DestructSolution {0, false};
		    it += 1;
		}
		
		current += 1;			
	    }
	    
	    if (current != left-> getExprs ().size ())
		return DestructSolution (0, false);
	    return soluce;
	    
	} else if (type-> is <IAggregateInfo> ()) {
	    return solveAggCst (left, right);
	}
	return DestructSolution (0, false);
    }
    
    DestructSolution DestructSolver::solveAggCst (StructCst left, Expression right) {
	DestructSolution soluce {0, true};
	if (!left-> getLeft ()-> is <IIgnore> ()) {
	    InfoType rtype = right-> info-> type ();
	    while (auto ref = rtype-> to <IRefInfo> ()) rtype = ref-> content ();
	    
	    auto ltype = left-> getLeft ()-> toType ();
	    if (ltype == NULL || !ltype-> info-> type ()-> is <IAggregateInfo> ()) return DestructSolution (0, false);	    
	    auto retType = rtype-> to <IAggregateInfo> ()-> isTyped (ltype-> info-> type ()-> to <IAggregateInfo> ());
	    
	    DestructSolution res {__VAR__, true};
	    res.test = new (Z0) IBinary (left-> token, right, ltype);
	    res.test-> info = new (Z0) ISymbol (left-> token, DeclSymbol::init (), res.test, retType);
	    
	    res.immutable = false;
	    if (!merge (soluce, res, Token::AND))
		return DestructSolution (0, false);
	}
	
	for (auto it : Ymir::r (0, left-> getExprs ().size ())) {
	    if (!left-> getExprs ()[it]-> is <INamedExpression> ()) {
		if (!left-> getLeft ()-> is <IIgnore> ()) 
		    Ymir::Error::aggMatchOnlyNamed (left-> getExprs ()[it]-> token);
		
		return DestructSolution (0, false);
	    }
	    auto named = left-> getExprs ()[it]-> to <INamedExpression> ();

	    auto dotExp = new (Z0) IDot ({named-> token, Token::DOT}, right, new (Z0) IVar (named-> token));
	    auto exp = (dotExp)-> expression (); 
	    //dotExp-> info = exp-> info;
	    if (exp == NULL) return DestructSolution (0, false);
	    
	    auto res = solve (named-> getValue (), exp);
	    if (!res.valid || !merge (soluce, res, Token::AND))
		return DestructSolution (0, false);
	}
	
	return soluce;
    }
    
    DestructSolution DestructSolver::solveTuple (ConstTuple left, Expression right) {
	InfoType type = right-> info-> type ();
	while (auto ref = type-> to <IRefInfo> ()) type = ref-> content ();	
	if (auto tuple = type-> to <ITupleInfo> ()) {
	    DestructSolution soluce {0, true};
	    if (left-> getExprs ().size () != tuple-> nbParams ()) {
		return DestructSolution {0, false};
	    }

	    for (auto it : Ymir::r (0, tuple-> nbParams ())) {
		auto exp = new (Z0) IExpand (right-> token, right, it);
		exp-> info = new (Z0) ISymbol (exp-> token, exp, tuple-> getParams ()[it]-> clone ());
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
	test-> value () = true;	
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
	    if (right-> info-> type ()-> is <IRefInfo> ()) {
		aux-> info = new (Z0) ISymbol (var-> token, aux, right-> info-> type ()-> clone ());
		Table::instance ().insert (aux-> info);
	    
		bin = (new (Z0) IAffectGeneric ({var-> token, Token::EQUAL},
						aux,
						right, false))-> expression ();
	    } else {
		aux-> info = new (Z0) ISymbol (var-> token, aux, new (Z0) IRefInfo (false, right-> info-> type ()-> clone ()));
		Table::instance ().insert (aux-> info);
		
		bin = (new (Z0) IAffectGeneric ({var-> token, Token::EQUAL},
						aux,
						right, true))-> expression ();
	    }
    	} else {
    	    aux-> info = new (Z0) ISymbol (var-> token, aux, new (Z0) IUndefInfo ());
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
    	test-> value () = true;
    	soluce.test = test-> expression ();
    	soluce.immutable = true;
	
    	return soluce;	
    }
    
        
    DestructSolution DestructSolver::solveNormal (Expression left, Expression right) {
	auto value = left-> expression ();
	if (value == NULL) return DestructSolution (0, false);

	auto rtype = right-> info-> type ();
	while (auto ref = rtype-> to <IRefInfo> ())
	    rtype = ref-> content ();
	
	if (!value-> info-> type ()-> CompOp (rtype))
	    return DestructSolution (0, false);
	
	Word tok {left-> token, Token::DEQUAL};
	if (rtype-> is <IPtrInfo> ()) {
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
	auto rtype = right-> info-> type ();
	while (auto ref = rtype-> to <IRefInfo> ())
	    rtype = ref-> content ();

	if (auto ref = ltype-> to <IRefInfo> ())
	    ltype = ref-> content ();
		
	if (ltype-> isSame (rtype)) {
	    auto var = new (Z0) IVar (left-> token);
	    var-> deco = left-> deco;
	    return solveVar (var, right, false);
	} else if ((ltype-> is<IAggregateCstInfo> () || ltype-> is <IAggregateInfo> ()) && rtype-> is<IAggregateInfo> ()) {
	    if (ltype-> is<IAggregateCstInfo> ())
		ltype = ltype-> TempOp ({});
	    
	    if (ltype-> CompOp (rtype) != NULL || rtype-> CompOp (ltype) != NULL) {
		DestructSolution soluce {__VAR__, true};
		//soluce.test = test;
		auto var = new (Z0) IVar (left-> token);
		auto varsoluce = solveVar (var, right, false);
		if (!varsoluce.valid || !merge (soluce, varsoluce, Token::AND))
		    return DestructSolution {0, false};
		return soluce;
	    }
	}
	return DestructSolution (0, false);
    }
    
    DestructSolution DestructSolver::solveBinary (Binary left, Expression right) {
	if (left-> token != Token::PIPE && left-> token != Token::TDOT && left-> token != Token::DDOT)
	    return solveNormal (left, right);

	if (left-> token == Token::PIPE) {
	    auto soluceLeft = solve (left-> getLeft (), right);
	    if (!soluceLeft.valid) return DestructSolution (0, false);
	    auto soluceRight = solve (left-> getRight (), right);
	    if (!soluceRight.valid || !merge (soluceLeft, soluceRight, Token::DPIPE))
		return DestructSolution (0, false);
	    return soluceLeft;
	} else {
	    Word tok = {left-> token, Keys::IN};
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
    }


    
}
