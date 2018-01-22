#include <ymir/semantic/pack/TemplateSolver.hh>
#include <ymir/ast/TypedVar.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    using namespace syntax;
    using namespace std;

    void print (vector <Expression> vec) {
	printf ("[");
	for (auto it : vec)
	    printf ("[%s], ", it-> token.getStr ().c_str ());
	printf ("]\n");	
    }

    TemplateSolver TemplateSolver::__instance__;
    
    TemplateSolver& TemplateSolver::instance () {
	return __instance__;
    }

    std::string TemplateSolution::toString () {
	Ymir::OutBuffer buf;
	buf.write ("{", this-> score, ", ", this-> valid);
	if (this-> type)
	    buf.write (this-> type-> typeString (), ", [");
	else buf.write ("null, [");
	for (auto it : this-> elements) {
	    buf.write (it.first, " : ", it.second-> info-> typeString ());
	}
	buf.write ("], [");
	for (auto it : this-> varTypes) {
	    buf.write (it-> typeString ());
	}
	buf.write ("]}");
	return buf.str ();	
    }
    
    TemplateSolver::TemplateSolver () {}
    
    bool TemplateSolver::merge (long& score, map <string, Expression>& left, TemplateSolution& right) {
	for (auto it : right.elements) {
	    auto inside = left.find (it.first);
	    if (inside != left.end ()) {
		auto ltype = inside-> second-> to<IType> ();
		auto rtype = it.second-> to<IType> ();
		if (!ltype || !rtype || !ltype-> info-> type-> isSame (rtype-> info-> type))
		    return false;
	    } else {
		left [it.first] = it.second;
	    }
	}
	score += right.score;
	return true;
    }

    bool TemplateSolver::merge (long&, map <string, Expression>& left, map <string, Expression>& right) {
	for (auto it : right) {
	    auto inside = left.find (it.first);
	    if (inside != left.end ()) {
		auto ltype = inside-> second-> to <IType> ();
		auto rtype = it.second-> to <IType> ();
		if (!ltype || !rtype || !ltype-> info-> type-> isSame (rtype-> info-> type))
		    return false;
	    } else {
		left [it.first] = it.second;
	    }
	}
	return true;	
    }
    
    TemplateSolution TemplateSolver::solve (vector <Expression> &tmps, Var param, InfoType type) {
	if (auto t = type-> to <IRefInfo> ()) type = t-> content ();	
	if (auto tvar = param-> to <ITypedVar> ()) {
	    if (tvar-> typeExp ()) return solve (tmps, tvar-> typeExp (), type);

	    bool isConst = false;				
	    auto typeVar = tvar-> typeVar ();
	    while (typeVar-> token == Keys::CONST) {
		isConst = true;
		typeVar = typeVar-> getTemplates () [0]-> to <IVar> ();
	    }

	    if (auto arr = typeVar-> to <IArrayVar> ()) {
		return solve (tmps, arr, type, isConst);
	    } else if (typeVar-> is <IType> ()) {
		return TemplateSolution (0, true, tvar-> typeVar ()-> info-> type);
	    } else {
		vector <Expression> types;
		TemplateSolution soluce (0, true);
		for (auto it : Ymir::r (0, typeVar-> getTemplates ().size ())) {
		    if (auto var = typeVar-> getTemplates () [it]-> to <IVar> ()) {
			if (!type-> getTemplate (it)) return TemplateSolution (0, false);
			auto typeTemplates = type-> getTemplate (it, typeVar-> getTemplates ().size () - (it + 1));
			auto res = this-> solveInside (tmps, var, typeTemplates);			
			if (!res.valid || !merge (soluce.score, soluce.elements, res))
			    return TemplateSolution (0, false);
			if (res.type)
			    types.push_back (new (GC) IType (var-> token, res.type));
			else
			    return TemplateSolution (0, false);			
		    }
		}

		for (auto it : tmps) {
		    if (auto var = it-> to <IVar> ()) {
			if (typeVar-> token.getStr () == var-> token.getStr ()) {
			    TemplateSolution res (0, true);
			    if (auto of = var-> to<IOfVar> ())
				res = solve (tmps, of, tvar, type, isConst);
			    else {
				res = solve (var, tvar, type, isConst);
			    }
			    if (!res.valid || !merge (soluce.score, soluce.elements, res))
				return TemplateSolution (0, false);
			    else {
				soluce.type = res.type;
				return soluce;
			    }			    
			}
		    }		
		}
		
		auto var = IVar (typeVar-> token, types);		
		soluce.type = (var).asType ()-> info-> type;
		soluce.type-> isConst (isConst);
		soluce.score += __VAR__;
		return soluce;
	    }
	}
	return TemplateSolution (0, false);
    }
    
    TemplateSolution TemplateSolver::solveInside (vector <Expression> &tmps, Var param, InfoType type) {
	if (auto t = type-> to <IRefInfo> ()) type = t-> content ();
	bool isConst = false;	    
	while (param-> token == Keys::CONST) {
	    isConst = true;
	    param = param-> getTemplates () [0]-> to <IVar> ();
	}		

	if (auto arr = param-> to <IArrayVar> ()) {
	    return solve (tmps, arr, type, isConst);
	} else {
	    vector <Expression> types;
	    TemplateSolution soluce (0, true);
	    for (auto it : Ymir::r (0, param-> getTemplates ().size ())) {
		if (auto var = param-> getTemplates () [it]-> to <IVar> ()) {
		    if (!type-> getTemplate (it)) return TemplateSolution (0, false);
		    auto typeTemplates = type-> getTemplate (it, param-> getTemplates ().size () - (it + 1));
		    auto res = this-> solveInside (tmps, var, typeTemplates);
		    if (!res.valid || !merge (soluce.score, soluce.elements, res))
			return TemplateSolution (0, false);
		    if (res.type)
			types.push_back (new (GC) IType (var-> token, res.type));
		    else
			return TemplateSolution (0, false);			
		}
	    }
	    
	    for (auto it : tmps) {
		if (auto var = it-> to <IVar> ()) {
		    if (param-> token.getStr () == var-> token.getStr ()) {
			TemplateSolution res (0, true);
			if (auto of = var-> to<IOfVar> ())
			    res = solve (tmps, of, param, type, isConst);
			else
			    res = solve (var, param, type, isConst);
			
			if (!res.valid || !merge (soluce.score, soluce.elements, res))
			    return TemplateSolution (0, false);
			else {
			    soluce.type = res.type;
			    return soluce;
			}			    
		    }
		}		
	    }
	    auto var = IVar (param-> token, types).asType ();
	    if (var == NULL) return TemplateSolution (0, false);
	    soluce.type = var-> info-> type;
	    soluce.type-> isConst (isConst);
	    soluce.score += __VAR__;
	    return soluce;
	}    
    }
	
    TemplateSolution TemplateSolver::solveInside (vector <Expression> &tmps, Var var, vector <InfoType> &type) {
	if (type.size () == 1) return solveInside (tmps, var, type [0]);
	else {
	    Ymir::Error::assert ("TODO");
	    return TemplateSolution (0, false);
	}
    }

    TemplateSolution TemplateSolver::solve (vector <Expression> &tmps, ArrayVar param, InfoType type, bool isConst) {
	if (!type-> is <IArrayInfo> ()) {
	    if (auto ptr = type-> to <IRefInfo> ()) {
		if (!ptr-> content ()-> is <IArrayInfo> ())
		    return TemplateSolution (0, false);
	    } else
		return TemplateSolution (0, false);
	}

	auto content = param-> contentExp ();
	auto type_ = type-> getTemplate (0);
	if (type_ == NULL) return TemplateSolution (0, false);

	TemplateSolution res (0, true);
	if (auto var = content-> to <IVar> ()) 
	    res = this-> solveInside (tmps, var, type_);
	else
	    res = this-> solveInside (tmps, content-> to<IFuncPtr> (), type_);
	
	if (res.valid) {
	    auto type_ = new (GC) IArrayInfo (isConst, res.type-> cloneOnExit ());
	    return TemplateSolution (res.score, true, type_, res.elements);
	}
	return TemplateSolution (0, false);	
    }

    TemplateSolution TemplateSolver::solve (vector <Expression> &, Expression , InfoType ) {
	Ymir::Error::assert ("TODO");
	return TemplateSolution (0, false);
    }

    TemplateSolution TemplateSolver::solveInside (vector <Expression> &, FuncPtr , InfoType ) {
	Ymir::Error::assert ("TODO");
	return TemplateSolution (0, false);
    }

    TemplateSolution TemplateSolver::solve (Var elem, Var param, InfoType type, bool isConst) {
	if (elem-> is<ITypedVar> ()) return TemplateSolution (0, false);
	else if (elem-> is<IArrayVar> ()) return TemplateSolution (0, false);

	if (elem-> token == Keys::CONST) {
	    return solve (elem-> getTemplates () [0]-> to<IVar> (), param, type, true);	    
	} else if (elem-> getDeco () == Keys::CONST)
	    isConst = true;
	
	auto type_ = type-> cloneOnExit ();
	type_-> isConst (isConst);
	map <string, Expression> ret;
	ret [elem-> token.getStr ()] = new (GC) IType (param-> token, type_);	
	return TemplateSolution (__VAR__, true, type_, ret);
    }

    TemplateSolution TemplateSolver::solve (Var elem, TypedVar param, InfoType type, bool isConst) {
	if (elem-> is<ITypedVar> ()) return TemplateSolution (0, false);
	else if (elem-> is<IArrayVar> ()) return TemplateSolution (0, false);
	else if (type-> is<IFunctionInfo> ()) return TemplateSolution (0, false);
	// TODO Object, Struct

	if (elem-> token == Keys::CONST) {
	    return solve (elem-> getTemplates () [0]-> to <IVar> (), param, type, true);	    
	} else if (elem-> getDeco () == Keys::CONST)
	    isConst = true;
	
	auto type_ = type-> cloneOnExit ();
	type_-> isConst (isConst);
	
	map <string, Expression> ret;	
	ret [elem-> token.getStr ()] = new (GC) IType (param-> token, type_);
	return TemplateSolution (__VAR__, true, type_, ret);
    }

    TemplateSolution TemplateSolver::solve (vector <Expression> &tmps, OfVar elem, Var param, InfoType type, bool isConst) {
	Var typeVar;
	auto typedParam = param-> to <ITypedVar> ();
	if (typedParam) typeVar = typedParam-> typeVar ();
	else typeVar = param;
		
	auto res = this-> solveInside (tmps, elem-> typeVar (), type);
	if (res.valid) {
	    if ((typedParam && typedParam-> getDeco () == Keys::CONST) || res.type-> isConst ())
		isConst = true;
	    res.type-> isConst (isConst);
	}
	
	if (!res.valid || !type-> CompOp (res.type))
	    return TemplateSolution (0, false);
	else {
	    auto type_ = res.type;//type-> cloneOnExit ();	    
	    type_-> isConst (isConst);
	    map<string, Expression> ret = {{elem-> token.getStr (), new (GC) IType (typeVar-> token, type_)}};
	    if (!merge (res.score, res.elements, ret))
		return TemplateSolution (0, false);
	    res.score += __VAR__;
	    return res;
	}
	
    }

    TemplateSolution TemplateSolver::solve (vector <Expression> &tmps, vector <Expression> &params) {
	TemplateSolution soluce (0, true);
	if (tmps.size () < params.size ())
	    return TemplateSolution (0, false);

	for (auto it : Ymir::r (0, params.size ())) {
	    TemplateSolution res (0, true);
	    if (auto v = tmps [it]-> to<IVar> ())
		res = this-> solveInside (tmps, v, params [it]);
	    else
		res = this-> solveInside (tmps, tmps [it], params [it]);

	    if (!res.valid || !merge (soluce.score, soluce.elements, res))
		return TemplateSolution (0, false);
	}
	return soluce;	
    }

    TemplateSolution TemplateSolver::solveInside (vector <Expression> &tmps, Var left, Expression right) {
	if (auto tvar = left-> to<ITypedVar> ())
	    return this-> solveInside (tmps, tvar, right);
	else if (auto of = left-> to <IOfVar> ())
	    return this-> solveInside (tmps, of, right);
	else if (auto type = right-> to<IType> ())
	    return this-> solveInside (left, type);
	else if (auto vvar = left-> to <IVariadicVar> ())
	    return this-> solveInside (tmps, vvar, right);
	else if (auto var = right-> to <IVar> ())
	    return this-> solveInside (left, var);
	else return TemplateSolution (0, false);
    }

    TemplateSolution TemplateSolver::solveInside (Var left, Type right) {
	auto type = right-> info-> type;
	auto clo = right-> clone ();
	clo-> info-> type = clo-> info-> type-> cloneOnExit ();
	map<string, Expression> value =  {{left-> token.getStr (), clo}};
	return TemplateSolution (__VAR__, true, type, value);
    }
    
    TemplateSolution TemplateSolver::solveInside (Var, Var) {
	//auto type = right-> info-> type;
	//if (auto cst = type-> to <StructCstInfo> ())
	// if (auto cst = type-> to <ObjectCstInfo> ())
	return TemplateSolution (0, false);
    }

    TemplateSolution TemplateSolver::solveInside (vector <Expression> &tmps, OfVar left, Expression right) {
	auto type = right-> to<IType> ();
	if (!type) return TemplateSolution (0, false);

	auto res = this-> solveInside (tmps, left-> typeVar (), type-> info-> type);
	if (!res.valid || !res.type-> CompOp (type-> info-> type))
	    return TemplateSolution (0, false);
	else {
	    auto clo = type-> clone ();
	    clo-> info-> type = clo-> info-> type-> cloneOnExit ();
	    res.type = type-> info-> type-> cloneOnExit ();
	    map <string, Expression> ret = {{left-> token.getStr (), clo}};
	    if (!merge (res.score, res.elements, ret))
		return TemplateSolution (0, false);
	    return res;
	}	
    }
    
    TemplateSolution TemplateSolver::solveInside (vector <Expression> & tmps, TypedVar left, Expression right) {
	auto type = right-> info-> type;
	if (!right-> info-> isImmutable ()) {
	    Ymir::Error::notImmutable (right-> info);
	    return TemplateSolution (0, false);
	}

	auto res = this-> solveInside (tmps, left-> typeVar (), type);
	if (!res.valid)
	    return TemplateSolution (0, false);

	if (!type-> isSame (res.type))
	    return TemplateSolution (0, false);

	map <string, Expression> ret = {{left-> token.getStr (), right}};
	if (!merge (res.score, res.elements, ret))
	    return TemplateSolution (0, false);
	return res;
    }

    TemplateSolution TemplateSolver::solveInside (vector <Expression> &, Expression , Expression ) {
	Ymir::Error::assert ("TODO");
	return TemplateSolution (0, false);
    }

    TemplateSolution TemplateSolver::solve (vector <Var> &tmps, vector <Expression> &params) {
	vector <Expression> aux;
	for (auto it : tmps)
	    aux.push_back (it);
	return solve (aux, params);
    }

    bool TemplateSolver::isValid (vector <Expression> &args) {
	for (auto it : args) {
	    if (auto var = it-> to <IVar> ()) {
		for (auto it_ : args) {
		    auto var_ = it_-> to<IVar> ();
		    if (it != it_ && var_ && var-> token.getStr () == var_-> token.getStr ())
			return false;
		}
	    }
	}
	return true;
    }

    bool TemplateSolver::isSolved (vector <Expression> &args, TemplateSolution soluce) {
	if (!soluce.valid) return false;
	return isSolved (args, soluce.elements);
    }

    bool TemplateSolver::isSolved (vector <Expression> &args, map <string, Expression> &types) {
	for (auto it : args) {
	    if (auto type = it-> to <ITypedVar> ()) {
		auto elem = types.find (type-> token.getStr ());
		if (elem != types.end () && elem-> second == NULL)
		    return false;
		else if (elem == types.end ()) return false;
	    } else {
		auto elem = types.find (it-> token.getStr ());
		if (elem != types.end () && elem-> second == NULL)
		    return false;
		else if (elem == types.end ()) return false;
	    }
	}
	return true;
    }
       
    vector <Expression> TemplateSolver::unSolved (vector <Expression> &args, TemplateSolution soluce) {
	std::vector <Expression> rets;
	ulong nb = 0;
	for (auto it : args) {
	    if (auto v = it-> to<ITypedVar> ()) {
		if (soluce.elements.find (v-> token.getStr ()) == soluce.elements.end ())
		    rets.push_back (it-> templateExpReplace (soluce.elements));
	    } else {
		Ymir::OutBuffer buf;
		buf.write (nb);
		if (soluce.elements.find (buf.str ()) == soluce.elements.end ())
		    rets.push_back (it-> templateExpReplace (soluce.elements));
	    }
	    nb ++;
	}
	return rets;
    }
    
}
