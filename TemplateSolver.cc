#include <ymir/semantic/pack/TemplateSolver.hh>
#include <ymir/ast/TypedVar.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    using namespace syntax;
    using namespace std;
    
    TemplateSolver TemplateSolver::__instance__;
    
    TemplateSolver& TemplateSolver::instance () {
	return __instance__;
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
	    if (tvar-> typeExp ()) {
		return solve (tmps, tvar-> typeExp (), type);
	    } else if (auto arr = tvar-> typeVar ()-> to <IArrayVar> ()) {
		return solve (tmps, arr, type);
	    } else if (tvar-> typeVar ()-> is <IType> ()) {
		return TemplateSolution (0, true, tvar-> typeVar ()-> info-> type);
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
				res = solve (tmps, of, tvar, type);
			    else
				res = solve (var, tvar, type);
			
			    if (!res.valid || !merge (soluce.score, soluce.elements, res))
				return TemplateSolution (0, false);
			    else {
				soluce.type = res.type;
				return soluce;
			    }			    
			}
		    }		
		}
	    	    
		soluce.type = (IVar (param-> token, types)).asType ()-> info-> type;
		soluce.score += __VAR__;
		return soluce;
	    }
	}
	return TemplateSolution (0, false);
    }
    
    TemplateSolution TemplateSolver::solveInside (vector <Expression> &tmps, Var param, InfoType type) {
	if (auto t = type-> to <IRefInfo> ()) type = t-> content ();
	if (auto arr = param-> to <IArrayVar> ()) {
	    return solve (tmps, arr, type);
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
			    res = solve (tmps, of, param, type);
			else
			    res = solve (var, param, type);
			
			if (!res.valid || !merge (soluce.score, soluce.elements, res))
			    return TemplateSolution (0, false);
			else {
			    soluce.type = res.type;
			    return soluce;
			}			    
		    }
		}		
	    }
	    	    
	    soluce.type = (IVar (param-> token, types)).asType ()-> info-> type;
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

    TemplateSolution TemplateSolver::solve (vector <Expression> &tmps, ArrayVar param, InfoType type) {
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
	if (res.valid)
	    return TemplateSolution (res.score, true, type-> cloneOnExit (), res.elements);
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

    TemplateSolution TemplateSolver::solve (Var elem, Var param, InfoType type) {
	if (elem-> is<ITypedVar> ()) return TemplateSolution (0, false);
	else if (elem-> is<IArrayVar> ()) return TemplateSolution (0, false);
	auto type_ = type-> cloneOnExit ();
	map <string, Expression> ret;
	ret [elem-> token.getStr ()] = new (GC) IType (param-> token, type_);
	return TemplateSolution (__VAR__, true, type_, ret);
    }

    TemplateSolution TemplateSolver::solve (Var elem, TypedVar param, InfoType type) {
	if (elem-> is<ITypedVar> ()) return TemplateSolution (0, false);
	else if (elem-> is<IArrayVar> ()) return TemplateSolution (0, false);
	else if (type-> is<IFunctionInfo> ()) return TemplateSolution (0, false);
	// TODO Object, Struct
	
	auto type_ = type-> cloneOnExit ();
	map <string, Expression> ret;
	ret [elem-> token.getStr ()] = new (GC) IType (param-> token, type_);
	return TemplateSolution (__VAR__, true, type_, ret);
    }

    TemplateSolution TemplateSolver::solve (vector <Expression> &tmps, OfVar elem, Var param, InfoType type) {
	Var typeVar;
	auto typedParam = param-> to <ITypedVar> ();
	if (typedParam) typeVar = typedParam-> typeVar ();
	else typeVar = param;

	auto res = this-> solveInside (tmps, elem-> typeVar (), type);
	if (res.valid) {
	    if (typedParam && typedParam-> getDeco () == Keys::CONST)
		res.type-> isConst (true);
	    else res.type-> isConst (false);
	}
	
	if (!res.valid || !type-> CompOp (res.type))
	    return TemplateSolution (0, false);
	else {
	    res.type = type;
	    map<string, Expression> ret = {{elem-> token.getStr (), new (GC) IType (typeVar-> token, type-> cloneOnExit ())}};
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
    
    TemplateSolution TemplateSolver::solveInside (vector <Expression> &, TypedVar , Expression ) {
	Ymir::Error::assert ("TODO");
	return TemplateSolution (0, false);
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
		if (elem != types.end () && elem-> second != NULL)
		    return true;
		return false;
	    } else {
		auto elem = types.find (it-> token.getStr ());
		if (elem != types.end () && elem-> second != NULL)
		    return true;
		return false;
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
