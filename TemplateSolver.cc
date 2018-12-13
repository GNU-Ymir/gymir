#include <ymir/semantic/pack/TemplateSolver.hh>
#include <ymir/semantic/object/TraitInfo.hh>
#include <ymir/ast/TypedVar.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/semantic/value/_.hh>
#include <ymir/ast/Tuple.hh>
#include <ymir/semantic/object/AggregateInfo.hh>

namespace semantic {
    using namespace syntax;
    using namespace std;

    void print (const vector<Expression> & vec) {
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
	    buf.write (", ", this-> type-> typeString (), ", [");
	else buf.write (", null, [");
	for (auto it : this-> elements) {
	    if (it.second-> info)
		buf.write (it.first, " : ", it.second-> info-> typeString ());
	    else if (auto params = it.second-> to<IParamList> ()) {
		buf.write (it.first, " : {"); 
		for (auto it : params-> getParamTypes ())
		    buf.write (it-> typeString (), " ");
		buf.write ("}");
	    }
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
		if (!ltype || !rtype) {
		    auto lpars = inside-> second-> to <IParamList> ();
		    auto rpars = it.second-> to <IParamList> ();
		    if (!lpars || !rpars || lpars-> getParams ().size () != rpars-> getParams ().size ()) return false;
		    for (auto it_ : Ymir::r (0, lpars-> getParams ().size ())) {
			auto linfo = lpars-> getParamTypes () [it_];
			auto rinfo = rpars-> getParamTypes () [it_];
			if (!linfo-> is <IUndefInfo> () &&
			    !rinfo-> is <IUndefInfo> () &&
			    !linfo-> isSame (rinfo)) {
			    if (auto ref = linfo-> to <IRefInfo> ()) {
				if (!rinfo-> isSame (ref-> content ()))
				    return false;
				else {
				    left [it.first] = inside-> second;
				}
			    } else if (auto ref = rinfo-> to <IRefInfo> ()) {
				if (!linfo-> isSame (ref-> content ()))
				    return false;
				else {
				    left [it.first] = it.second;
				}
			    } else {		
				return false;
			    }
			}
		    }
		} else if (!ltype-> info-> type ()-> is <IUndefInfo> () &&
			   !rtype-> info-> type ()-> is <IUndefInfo> () &&
			   !ltype-> info-> type ()-> isSame (rtype-> info-> type ())) {
		    if (auto ref = ltype-> info-> type ()-> to <IRefInfo> ()) {
			if (!rtype-> info-> type ()-> isSame (ref-> content ()))
			    return false;
			else {
			    left [it.first] = inside-> second;
			}
		    } else if (auto ref = rtype-> info-> type ()-> to <IRefInfo> ()) {
			if (!ltype-> info-> type ()-> isSame (ref-> content ()))
			    return false;
			else {
			    left [it.first] = it.second;
			}
		    } else {		
			return false;
		    }
		}
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
		if (!ltype || !rtype) {
		    auto lpars = inside-> second-> to <IParamList> ();
		    auto rpars = it.second-> to <IParamList> ();
		    if (!lpars || !rpars || lpars-> getParams ().size () != rpars-> getParams ().size ()) return false;
		    for (auto it_ : Ymir::r (0, lpars-> getParams ().size ())) {
			auto linfo = lpars-> getParamTypes () [it_];
			auto rinfo = rpars-> getParamTypes () [it_];
			if (!linfo-> is <IUndefInfo> () &&
			    !rinfo-> is <IUndefInfo> () &&
			    !linfo-> isSame (rinfo)) {
			    if (auto ref = linfo-> to <IRefInfo> ()) {
				if (!rinfo-> isSame (ref-> content ()))
				    return false;
				else {
				    left [it.first] = inside-> second;
				}
			    } else if (auto ref = rinfo-> to <IRefInfo> ()) {
				if (!linfo-> isSame (ref-> content ()))
				    return false;
				else {
				    left [it.first] = it.second;
				}
			    } else return false;
			}
		    }
		} else if (!ltype-> info-> type ()-> is <IUndefInfo> () &&
			   !rtype-> info-> type ()-> is <IUndefInfo> () &&
			   !ltype-> info-> type ()-> isSame (rtype-> info-> type ())) {
		    if (auto ref = ltype-> info-> type ()-> to <IRefInfo> ()) {
			if (!rtype-> info-> type ()-> isSame (ref-> content ()))
			    return false;
			else {
			    left [it.first] = inside-> second;
			}
		    } else if (auto ref = rtype-> info-> type ()-> to <IRefInfo> ()) {
			if (!ltype-> info-> type ()-> isSame (ref-> content ()))
			    return false;
			else {
			    left [it.first] = it.second;
			}
		    } else {		
			return false;
		    }
		}
	    } else {
		left [it.first] = it.second;
	    }
	}
	return true;	
    }
    
    TemplateSolution TemplateSolver::solve (const vector <Expression> &tmps, Var param, InfoType type) {	
	Ymir::log ("Solve var : ", tmps, "|", param, "|", type);
	if (auto t = type-> to <IRefInfo> ()) type = t-> content ();
	if (auto tvar = param-> to <ITypedVar> ()) {
	    if (tvar-> typeExp ()-> is <IFuncPtr> ()) {
		return solve (tmps, tvar-> typeExp (), type);
	    }

	    bool isConst = false;
	    Expression typeExp = tvar-> typeExp ();
	    auto typeVar = tvar-> typeExp ()-> to <IVar> ();
	    if (typeVar) {
		while (typeVar && typeVar-> token == Keys::CONST) {
		    isConst = true;
		    typeExp = typeVar-> getTemplates () [0];
		    typeVar = typeExp-> to <IVar> ();
		}
	    }

	    if (auto ddot = typeExp-> to <IDColon> ()) {
		return solve (tmps, ddot, type, isConst);
	    } else if (auto carr = typeExp-> to <IConstArray> ()) {
		return solve (tmps, carr, type, isConst);
	    } else if (auto arr = typeExp-> to <IArrayVar> ()) {
		return solve (tmps, arr, type, isConst);
	    } else if (auto all = typeExp-> to <IArrayAlloc> ()) {
		return solve (tmps, all, type, isConst);
	    } else if (typeExp-> isType ()) {
		return TemplateSolution (0, true, tvar-> typeExp ()-> info-> type ());
	    } else if (typeVar != NULL) {
		vector <Expression> types;
		TemplateSolution soluce (0, true);
		for (auto it : Ymir::r (0, typeVar-> getTemplates ().size ())) {
		    auto var = typeVar-> getTemplates () [it];
		    if (!type-> getTemplate (it)) return TemplateSolution (0, false);
		    auto typeTemplates = type-> getTemplate (it, typeVar-> getTemplates ().size () - (it + 1));
		    auto res = this-> solveInside (tmps, var, typeTemplates);			
		    if (!res.valid || !merge (soluce.score, soluce.elements, res))
			return TemplateSolution (0, false);
		    if (res.type)
			types.push_back (new (Z0)  IType (var-> token, res.type));
		    else if (typeTemplates.size () != 1) {
			for (auto it_ : res.elements) {
			    if (auto params = it_.second-> to<IParamList> ()) {
				for (auto it__ : params-> getParams ())
				    types.push_back (it__);
			    } else Ymir::Error::assert ("!!");
			}
		    } else
			return TemplateSolution (0, false);			
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
		
		auto var = (new (Z0) IVar (typeVar-> token, types))-> asType ();
		if (!var) return TemplateSolution (0, false);
		soluce.type = var-> info-> type ();
		soluce.type-> isConst (isConst);
		soluce.score += __VAR__;
		return soluce;
	    }
	}
	return TemplateSolution (0, false);
    }
    
    TemplateSolution TemplateSolver::solveInside (const vector <Expression> & tmps, Expression param, InfoType type) {
	Ymir::log ("Solve inside expr 1 : ", tmps, "|", param, "|", type);
	bool isConst = false;
	auto tvar = param-> to <IVar> ();
	while (tvar && tvar-> token == Keys::CONST) {
	    isConst = true;
	    param = tvar-> getTemplates () [0];
	    tvar = param-> to <IVar> ();
	}

	if (auto var = param-> to <IVar> ())
	    return solveInside (tmps, var, type);
	else if (auto ddot = param-> to <IDColon> ()) 
	    return solve (tmps, ddot, type, isConst);
	else if (auto all = param-> to <IArrayAlloc> ()) 
	    return solve (tmps, all, type, isConst);
	else if (auto carr = param-> to <IConstArray> ())
	    return solve (tmps, carr, type, isConst);
	else if (auto var = param-> to <IArrayVar> ())
	    return solve (tmps, var, type, isConst);
	else if (auto fn = param-> to <IFuncPtr> ())
	    return solveInside (tmps, fn, type);
	return TemplateSolution (0, false);
    }

    TemplateSolution TemplateSolver::solveInside (const vector <Expression> & tmps, Expression param, const std::vector <InfoType>& types) {
	Ymir::log ("Solve inside expr 2 : ", tmps, "|", param, "|", types);
	bool isConst = false;
	auto tvar = param-> to <IVar> ();
	while (tvar && tvar-> token == Keys::CONST) {
	    isConst = true;	    
	    if (tvar-> getTemplates ().size () != 0) {
		param = tvar-> getTemplates () [0];
		tvar = param-> to <IVar> ();
	    } else {
		break;
	    }
	}

	if (auto var = param-> to <IVar> ())
	    return solveInside (tmps, var, types);
	else if (auto ddot = param-> to <IDColon> ()) {
	    if (types.size () == 1)
		return solve (tmps, ddot, types [0], isConst);
	} else if (auto all = param-> to <IArrayAlloc> ()) {
	    if (types.size () == 1) 
		return solve (tmps, all, types [0], isConst);
	} else if (auto var = param-> to <IArrayVar> ()) {
	    if (types.size () == 1)
		return solve (tmps, var, types [0], isConst);
	} else if (auto carr = param-> to <IConstArray> ()) {
	    if (types.size () == 1)
		return solve (tmps, carr, types [0], isConst);
	} else if (auto fn = param-> to <IFuncPtr> ())
	    return solveInside (tmps, fn, types);
	return TemplateSolution (0, false);
    }
    
    TemplateSolution TemplateSolver::solveInside (const vector <Expression> &tmps, Var param, InfoType type) {
	Ymir::log ("Solve inside var : ", tmps, "|", param, "|", type);
	bool isConst = false;	    
	while (param && param-> token == Keys::CONST) {
	    isConst = true;
	    if (param-> getTemplates ().size () != 0) 
		param = param-> getTemplates () [0]-> to <IVar> ();
	    else break;
	}
	if (param == NULL) return TemplateSolution (0, false);

	if (auto carr = param-> to <IConstArray> ()) {
	    return solve (tmps, carr, type, isConst);
	} else if (auto arr = param-> to <IArrayVar> ()) {
	    return solve (tmps, arr, type, isConst);
	} else if (auto all = param-> to <IArrayAlloc> ()) {
	    return solve (tmps, all, type, isConst);
	} else if (auto ty = param-> to <IType> ()) {
	    TemplateSolution soluce (0, true);
	    soluce.type = ty-> info-> type ()-> clone ();
	    soluce.type-> isConst (isConst);
	    if (!ty-> info-> type ()-> isSame (type) && !type-> is<IUndefInfo> ())
		return TemplateSolution (0, false);
	    soluce.score += __VAR__;
	    return soluce;
	} else {
	    
	    vector <Expression> types;	    
	    TemplateSolution soluce (0, true);
	    for (auto it : Ymir::r (0, param-> getTemplates ().size ())) {
		auto var = param-> getTemplates () [it];
		if (!type-> getTemplate (it)) return TemplateSolution (0, false);
		auto typeTemplates = type-> getTemplate (it, param-> getTemplates ().size () - (it + 1));
		auto res = this-> solveInside (tmps, var, typeTemplates);
		if (!res.valid || !merge (soluce.score, soluce.elements, res))
		    return TemplateSolution (0, false);
		if (res.type)
		    types.push_back (new (Z0)  IType (var-> token, res.type));
		else if (typeTemplates.size () != 1) {
		    for (auto it_ : res.elements) {
			if (auto params = it_.second-> to<IParamList> ()) {
			    for (auto it__ : params-> getParams ())
				types.push_back (it__-> to <IType> ());
			} else Ymir::Error::assert ("!!");
		    }
		} else
		    return TemplateSolution (0, false);					
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
	    
	    auto var = (new (Z0) IVar (param-> token, types))-> asType ();
	    if (var == NULL) return TemplateSolution (0, false);

	    if (!var-> info-> type ()-> isSame (type) && !type-> is<IUndefInfo> ())
		return TemplateSolution (0, false);
	    soluce.type = var-> info-> type ();
	    soluce.type-> isConst (isConst);
	    soluce.score += __VAR__;
	    return soluce;
	}    
    }
	
    TemplateSolution TemplateSolver::solveInside (const vector <Expression> &tmps, Var var, const vector <InfoType> &type) {
	Ymir::log ("Solve inside var ... : ", tmps, "|", var, "|", type);
	if (type.size () == 1) return solveInside (tmps, var, type [0]);
	else {
	    for (auto it : tmps) {
		if (auto vvar = it-> to <IVariadicVar> ()) {
		    if (var-> token.getStr () == vvar-> token.getStr ()) {
			TemplateSolution res = TemplateSolution (0, true);
			res.varTypes = type;
			std::vector <Expression> types;
			for (auto it : Ymir::r (0, type.size ())) {
			    Word token {var-> token, Ymir::OutBuffer ("_", it, var-> token.getStr ()).str ()};
			    types.push_back (new (Z0) IType (token, type [it]));
			}
			res.elements [var-> token.getStr ()] = new (Z0) IParamList (var-> token, types);
			return res;
		    }
		}
	    }
	    return TemplateSolution (0, false);
	}
    }

    TemplateSolution TemplateSolver::solve (const vector <Expression> &tmps, DColon param, InfoType type, bool isConst) {
	Ymir::log ("Solve DColon : ", tmps, "|", param, "|", type, "|", isConst);
	bool needToClose = false;
	auto space = Table::instance ().space ();
	auto left = param-> getLeft ()-> expression ();
	if (left == NULL) return TemplateSolution (0, false);
	
	auto mod = left-> info-> type ()-> to <IModuleInfo> ();
	if (mod == NULL) return TemplateSolution (0, false);
	auto content = mod-> get ();
	if (content == NULL) return TemplateSolution (0, false);    

	if (!content-> authorized (space)) {
	    content-> addOpen (space);
	    needToClose = true;
	}
	
	auto right = param-> getRight ();
	TemplateSolution res (0, false);
	if (auto var = right-> to <IVar> ()) {
	    res = this-> solveInside (tmps, var, type);
	} else if (auto dot = right-> to <IDColon> ()) {
	    res = this-> solve (tmps, dot, type, isConst);
	} 

	if (needToClose) content-> close (space);
	if (res.valid && res.type != NULL)
	    res.type-> isConst (isConst);
	
	return res;
    }
    
    TemplateSolution TemplateSolver::solve (const vector <Expression> &tmps, ConstArray param, InfoType type, bool isConst) {
	Ymir::log ("Solve ConstArray : ", tmps, "|", param, "|", type, "|", isConst);
	if (param-> getNbParams () != 1) return TemplateSolution (0, false);
	if (!type-> is <IArrayInfo> ()) {
	    if (auto ptr = type-> to<IRefInfo> ()) {
		if (!ptr-> content ()-> is <IArrayInfo> ())
		    return TemplateSolution (0, false);		
	    } else return TemplateSolution (0, false);
	}
	
	auto content = param-> getParam (0);
	auto type_ = type-> getTemplate (0);
	if (type_ == NULL) return TemplateSolution (0, false);

	TemplateSolution res (0, true);
	if (auto var = content-> to <IVar> ()) 
	    res = this-> solveInside (tmps, var, type_);
	else if (content-> is <IFuncPtr> ())
	    res = this-> solveInside (tmps, content-> to<IFuncPtr> (), type_);
	else return TemplateSolution (0, false);
	if (!res.valid) return TemplateSolution (0, false);
	
	type_ = new (Z0)  IArrayInfo (isConst, res.type-> cloneOnExit ());
	return TemplateSolution (res.score, true, type_, res.elements);
    }
    
    TemplateSolution TemplateSolver::solve (const vector <Expression> &tmps, ArrayVar param, InfoType type, bool isConst) {
	Ymir::log ("Solve ArrayVar : ", tmps, "|", param, "|", type, "|", isConst);
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
	if (!res.valid) return TemplateSolution (0, false);
	if (!param-> getLen ()) {
	    auto type_ = new (Z0)  IArrayInfo (isConst, res.type-> cloneOnExit ());
	    return TemplateSolution (res.score, true, type_, res.elements);
	} else {
	    auto innerType = res.type;
	    auto paramSize = param-> getLen ();
	    auto size = type-> getTemplate (1);
	    
	    if (size == NULL) return TemplateSolution (0, false);
	    if (auto temp = paramSize-> to <IVar> ()) {
		TemplateSolution res2 (0, false);
		paramSize = paramSize-> templateExpReplace ({});
		paramSize-> info = new (Z0) ISymbol (paramSize-> token, paramSize, size);
		for (auto it : Ymir::r (0, tmps.size ())) {
		    if (auto v = tmps [it]-> to<IVar> ()) {
			if (temp-> token.getStr () == v-> token.getStr ()) {
			    res2 = this-> solveInside (tmps, v, paramSize);
			    break;
			}
		    }
		}

		if ((!res2.valid || !merge (res.score, res.elements, res2)))
		    return TemplateSolution (0, false);

		auto arraySize = size-> value ()-> to<IFixedValue> ()-> getUValue ();
		auto type_ = new (Z0) IArrayInfo (isConst, innerType-> cloneOnExit ());
		type_-> isStatic (true, arraySize);
		return TemplateSolution (res.score, true, type_, res.elements);
	    } else {
		auto arraySize = paramSize-> expression ()-> info-> value ()-> to <IFixedValue> ()-> getUValue ();
		auto type_ = new (Z0) IArrayInfo (isConst, innerType-> cloneOnExit ());
		type_-> isStatic (true, arraySize);
		return TemplateSolution (res.score, true, type_, res.elements);
	    }

	}
    }


    TemplateSolution TemplateSolver::solve (const vector <Expression> & tmps, ArrayAlloc param, InfoType type, bool isConst) {
	Ymir::log ("Solve ArrayAlloc : ", tmps, "|", param, "|", type, "|", isConst);
	if (!type-> is<IArrayInfo> ()) {
	    if (auto ptr = type-> to<IRefInfo> ()) {
		if (!ptr-> content ()-> is <IArrayInfo> ()) {
		    return TemplateSolution (0, false);
		} 
	    } else return TemplateSolution (0, false);
	}

	auto content = param-> getType ();
	auto type_ = type-> getTemplate (0);
	if (type_ == NULL) return TemplateSolution (0, false);

	TemplateSolution res (0, true);
	if (auto var = content-> to <IVar> ()) {
	    res = this-> solveInside (tmps, var, type_);
	} else {
	    res = this-> solveInside (tmps, content-> to <IFuncPtr> (), type_);
	}
	
	if (!res.valid) return TemplateSolution (0, false);
	auto innerType = res.type;
	auto paramSize = param-> getSize ();
	auto size = type-> getTemplate (1);

	if (size == NULL) return TemplateSolution (0, false);
	if (auto temp = paramSize-> to <IVar> ()) {
	    TemplateSolution res2 (0, false);
	    paramSize = paramSize-> templateExpReplace ({});
	    paramSize-> info = new (Z0) ISymbol (paramSize-> token, paramSize, size);
	    for (auto it : Ymir::r (0, tmps.size ())) {
		if (auto v = tmps [it]-> to<IVar> ()) {
		    if (temp-> token.getStr () == v-> token.getStr ()) {
			res2 = this-> solveInside (tmps, v, paramSize);
			break;
		    }
		}
	    }

	    if ((!res2.valid || !merge (res.score, res.elements, res2)))
		return TemplateSolution (0, false);

	    auto arraySize = size-> value ()-> to<IFixedValue> ()-> getUValue ();
	    auto type_ = new (Z0) IArrayInfo (isConst, innerType-> cloneOnExit ());
	    type_-> isStatic (true, arraySize);
	    return TemplateSolution (res.score, true, type_, res.elements);
	} else {
	    auto arraySize = paramSize-> expression ()-> info-> value ()-> to <IFixedValue> ()-> getUValue ();
	    auto type_ = new (Z0) IArrayInfo (isConst, innerType-> cloneOnExit ());
	    type_-> isStatic (true, arraySize);
	    return TemplateSolution (res.score, true, type_, res.elements);
	}	
    }
    
    TemplateSolution TemplateSolver::solve (const vector <Expression> & tmps, Expression elem, InfoType type) {
	Ymir::log ("Solve expr : ", tmps, "|", elem, "|", type);
	if (auto func = elem-> to <IFuncPtr> ()) {
	    return solveInside (tmps, func, type);
	}
	return TemplateSolution (0, false);
    }

    TemplateSolution TemplateSolver::solveInside (const vector <Expression> & tmps, FuncPtr func, InfoType type) {
	Ymir::log ("Solve inside fn : ", tmps, "|", func, "|", type);
	vector <Expression> types;
	TemplateSolution soluce (0, true);
	// if (type-> nbTemplates () != func-> getParams ().size () + 1)
	//     return TemplateSolution (0, false);

	ulong nb = 0;
	for (auto it : Ymir::r (0, func-> getParams ().size ())) {
	    auto current = func-> getParams () [it];	    
	    if (!type-> getTemplate (it)) return TemplateSolution (0, false);
	    auto typeTemplates = type-> getTemplate (it, func-> getParams ().size () - (it + 1));
	    
	    nb += typeTemplates.size ();
	    TemplateSolution res (0, true);
	    if (auto var = current-> to<IVar> ())
		res = this-> solveInside (tmps, var, typeTemplates);
	    else {
		if (typeTemplates.size () != 1) return TemplateSolution (0, false);
		res = this-> solveInside (tmps, current, typeTemplates [0]);
	    }
	    
	    if (!res.valid || !merge (soluce.score, soluce.elements, res))
		return TemplateSolution (0, false);

	    if (res.type)
		types.push_back (new (Z0)  IType (current-> token, res.type));
	    else if (typeTemplates.size () != 1) {
		for (auto it_ : res.elements) {
		    if (auto params = it_.second-> to<IParamList> ()) {
			for (auto it__ : params-> getParams ())
			    types.push_back (it__-> to <IType> ());
		    } else Ymir::Error::assert ("!!");
		}
	    } else
		return TemplateSolution (0, false);			
	}

	auto typeTemplate = type-> getTemplate (nb);
	if (typeTemplate == NULL) return TemplateSolution (0, false);
	auto res = this-> solveInside (tmps, func-> getRet (), typeTemplate);
	
	if (!res.valid || !merge (soluce.score, soluce.elements, res))
	    return TemplateSolution (0, false);
	if (res.type) {
	    auto var = (new (Z0) IFuncPtr (func-> token, types,
					  new (Z0) IType (func-> getRet ()-> token, res.type)
	    ))-> expression ();

	    if (var == NULL) return TemplateSolution (0, false);
	    soluce.type = var-> info-> type ();
	    soluce.type-> isConst (false);
	    soluce.score += __VAR__;
	    return soluce;
	}
	return TemplateSolution (0, false);
    }

    TemplateSolution TemplateSolver::solve (Var elem, Var param, InfoType type, bool isConst) {
	Ymir::log ("Solve var uniq : ", elem, "|", param, "|", type, "|", isConst);
	if (elem-> is<ITypedVar> ()) return TemplateSolution (0, false);
	else if (elem-> is <IConstArray> ()) return TemplateSolution (0, false);
	else if (elem-> is <IArrayVar> ()) return TemplateSolution (0, false);
	else if (elem-> is <IArrayAlloc> ()) return TemplateSolution (0, false);

	if (elem-> token == Keys::CONST) {
	    return solve (elem-> getTemplates () [0]-> to<IVar> (), param, type, true);	    
	} else if (elem-> getDeco () == Keys::CONST)
	    isConst = true;
	
	auto type_ = type-> cloneOnExit ();
	type_-> isConst (isConst);
	map <string, Expression> ret;
	ret [elem-> token.getStr ()] = new (Z0)  IType (param-> token, type_);	
	return TemplateSolution (__VAR__, true, type_, ret);
    }

    TemplateSolution TemplateSolver::solve (Var elem, TypedVar param, InfoType type, bool isConst) {
	Ymir::log ("Solve typed var uniq : ", elem, "|", param, "|", type, "|", isConst);
	if (elem-> is<ITypedVar> ()) return TemplateSolution (0, false);
	else if (elem-> is <IConstArray> ()) return TemplateSolution (0, false);
	else if (elem-> is<IArrayVar> ()) return TemplateSolution (0, false);
	else if (elem-> is<IArrayAlloc> ()) return TemplateSolution (0, false);
	else if (type-> is<IFunctionInfo> ()) return TemplateSolution (0, false);	
	// TODO Object, Struct

	if (elem-> token == Keys::CONST) {
	    return solve (elem-> getTemplates () [0]-> to <IVar> (), param, type, true);	    
	} else if (elem-> getDeco () == Keys::CONST)
	    isConst = true;
	
	auto type_ = type-> cloneOnExit ();
	if (auto fn = type_-> to <IFunctionInfo> ()) {
	    if (fn-> isLambda ()) {
		type_ = fn-> toPtr (param-> token);
		if (type_ == NULL) return TemplateSolution (0, false);
	    }
	}
	
	type_-> isConst (isConst);
	
	map <string, Expression> ret;	
	ret [elem-> token.getStr ()] = new (Z0)  IType (param-> token, type_);
	return TemplateSolution (__VAR__, true, type_, ret);
    }

    TemplateSolution TemplateSolver::solve (const vector <Expression> &tmps, OfVar elem, Var param, InfoType type, bool isConst) {
	Ymir::log ("Solve OfVar : ", tmps, "|", elem, "|", param, "|", type, "|", isConst);
	Expression typeVar;
	auto typedParam = param-> to <ITypedVar> ();
	if (typedParam) typeVar = typedParam-> typeExp ();
	else typeVar = param;
	if (elem-> isTrait ()) {
	    auto traitInfo = elem-> getTypeVar ()-> expression ();
	    if (traitInfo == NULL || !traitInfo-> info-> type ()-> is<semantic::ITraitInfo> ()) {
		Ymir::Error::useAsTrait (elem-> getTypeVar ()-> token);
		return TemplateSolution (0, false);
	    }

	    if (!traitInfo-> info-> type ()-> to <ITraitInfo> ()-> validate (type))
		return TemplateSolution (0, false);
	    auto type_ = type-> cloneOnExit ();
	    if ((typedParam && typedParam-> getDeco () == Keys::CONST) || type-> isConst ())
		isConst = true;
	    type_-> isConst (isConst);
	    map<string, Expression> ret = {{elem-> token.getStr (), new (Z0)  IType (typeVar-> token, type_)}};
	    	    
	    return TemplateSolution {__VAR__, true, type, ret};
	} else {
	    auto res = this-> solveInside (tmps, elem-> getTypeVar (), type);
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
		map<string, Expression> ret = {{elem-> token.getStr (), new (Z0)  IType (typeVar-> token, type_)}};
		if (!merge (res.score, res.elements, ret))
		    return TemplateSolution (0, false);
		res.score += __VAR__;
		return res;
	    }
	}
	
    }

    Expression getAndRemove (std::string elem, std::map <std::string, Expression> & elements) {
	auto it = elements.find (elem);
	if (it == elements.end ()) return NULL;
	elements.erase (elem);
	return it-> second;
    }

    TemplateSolution TemplateSolver::solveVariadicValue (VariadicVar last, const std::vector <Expression>& params) {
	Ymir::log ("Solve variadic value : ", last, "|", params);
	map <string, Expression> ret;
	std::vector <Expression> expParams;
	std::vector <InfoType> types;
	for (auto it : Ymir::r (0, params.size ())) {
	    if (!params [it]-> info-> isImmutable ()) {
		Ymir::Error::notImmutable (params [it]-> token, params [it]-> info);
		return TemplateSolution (0, false);
	    }

	    types.push_back (params [it]-> info-> type ()-> cloneOnExit ());
	    expParams.push_back (params [it]-> info-> value ()-> toYmir (params [it]-> info));
	}
	
	auto type = new (Z0) ITupleInfo (true, true);
	type-> getParams () = types;
	Word begTok {last-> token, Token::LPAR}, endTok {last-> token, Token::RPAR};
	auto ctuple = new (Z0) IConstTuple (begTok, endTok, expParams);
	ctuple-> isFake () = true;
	ret [last-> token.getStr ()] = ctuple;
	
	return TemplateSolution {__VAR__, true, type, ret};
    }
    
    TemplateSolution TemplateSolver::solveVariadic (const std::vector <Expression> tmps, Expression last, std::vector <Expression> params) {
	Ymir::log ("Solve variadic exprs : ", tmps, "|", last, "|", params);
	if (auto var = last-> to <IVariadicVar> ()) {
	    if (var-> isValue ())
		return solveVariadicValue (var, params);
	}
	if (params.size () == 1) {
	    auto var = new (Z0) IVar (last-> token);
	    return solveInside (tmps, var, params [0]);
	} else {
	    TemplateSolution soluce (0, true);
	    std::vector <Expression> elements;
	    for (auto it : Ymir::r (0, params.size ())) {
		Word token {last-> token, Ymir::OutBuffer ("_", it, last-> token.getStr ()).str ()};
		auto var = new (Z0) IVar (token);
		auto res = solveInside (tmps, var, params [it]);
		if (!res.valid || !merge (soluce.score, soluce.elements, res))
		    return TemplateSolution (0, false);
		elements.push_back (getAndRemove (token.getStr (), soluce.elements));
	    }
	    
	    auto aux = new (Z0) IParamList (last-> token, elements);
	    map<string, Expression> ret = {{last-> token.getStr (), aux}};
	    if (!merge (soluce.score, soluce.elements, ret))
		return TemplateSolution (0, false);
	    //soluce.score += __VAR__;
	    soluce.isVariadic = true;
	    return soluce;
	}
    }

    TemplateSolution TemplateSolver::solveVariadic (const std::vector <syntax::Expression> tmps, syntax::Expression last, const std::vector <InfoType>& params) {
	Ymir::log ("Solve variadic : ", tmps, "|", last, "|", params);
	if (params.size () == 1) {
	    auto var = new (Z0) IVar (last-> token);
	    return solveInside (tmps, var, params [0]);
	} else {
	    TemplateSolution soluce (0, true);
	    while (last && last-> is <IVar> () && last-> token == Keys::CONST) {
		last = last-> to<IVar> ()-> getTemplates () [0]-> to <IVar> ();
	    }

	    std::vector <Expression> elements;	    
	    for (auto it : Ymir::r (0, params.size ())) {
		Word token {last-> token, Ymir::OutBuffer ("_", it, last-> token.getStr ()).str ()};
		auto var = new (Z0) IVar (token);
		auto res = solveInside (tmps, var, new (Z0) IType (token, params [it]-> clone ()));
		if (res.valid) res.type-> isConst (params[it]-> isConst ());
		if (!res.valid || !merge (soluce.score, soluce.elements, res))
		    return TemplateSolution (0, false);
		elements.push_back (getAndRemove (token.getStr (), soluce.elements));
	    }
	    
	    auto aux = new (Z0) IParamList (last-> token, elements);
	    map<string, Expression> ret = {{last-> token.getStr (), aux}};
	    if (!merge (soluce.score, soluce.elements, ret))
		return TemplateSolution (0, false);

	    //	    soluce.score += __VAR__;
	    soluce.isVariadic = true;
	    return soluce;
	}
    } 
    
    TemplateSolution TemplateSolver::solveVariadic (const vector <Expression> & tmps, const vector <Expression> & params) {
	Ymir::log ("Solve variadic : ", tmps, "|", params);
	auto last = tmps [tmps.size () - 1];
	std::vector <Expression> tmps2 (tmps.begin (), tmps.end () - 1);

	std::vector <Expression> aux, params2;
	if (params.size () > tmps2.size ()) {
	    aux = std::vector <Expression> (params.end () - (params.size () - tmps2.size ()), params.end ());	
	    params2  = std::vector <Expression> (params.begin (), params.end () - (params.size () - tmps2.size ()));
	} else {
	    params2 = params;
	}
	
	TemplateSolution soluce (0, true);
	for (auto it : Ymir::r (0, params2.size ())) {
	    TemplateSolution res (0, true);
	    if (auto v = tmps2 [it]-> to<IVar> ()) {
		res = this-> solveInside (tmps2, v, params2 [it]);
	    } else {
		res = this-> solveInside (tmps2, tmps2 [it], params2 [it]);
	    }
	    
	    if (!res.valid || !merge (soluce.score, soluce.elements, res))
		return TemplateSolution (0, false);
	}

	if (aux.size () != 0) {
	    auto res = solveVariadic (tmps2, last, aux);
	    if (!res.valid || !merge (soluce.score, soluce.elements, res))
		return TemplateSolution (0, false);
	}
	
	return soluce;	
    }
    
    TemplateSolution TemplateSolver::solve (const vector <Expression> &tmps, const vector <Expression> &params) {
	Ymir::log ("Solve : ", tmps, "|", params);
	TemplateSolution soluce (0, true);
	if (tmps.size () != 0 && tmps [tmps.size () - 1]-> is <IVariadicVar> ()) {
	    return solveVariadic (tmps, params);
	} else if (tmps.size () < params.size ()) {
	    return TemplateSolution (0, false);
	}

	bool end = false;
	for (auto it : Ymir::r (0, params.size ())) {
	    TemplateSolution res (0, true);
	     if (auto vv = tmps [it]-> to <IVariadicVar> ()) {
		std::vector<Expression> rest (params.begin () + it, params.end ());
		res = this-> solveVariadic (tmps, vv, rest);
		end = true;
	    } else if (auto v = tmps [it]-> to<IVar> ()) {
		res = this-> solveInside (tmps, v, params [it]);
	    } else {
		res = this-> solveInside (tmps, tmps [it], params [it]);
	    }
	    
	    if (!res.valid || !merge (soluce.score, soluce.elements, res))
		return TemplateSolution (0, false);
	    if (end) break;
	}
	return soluce;	
    }
    
    TemplateSolution TemplateSolver::solveInside (const vector <Expression> &tmps, Var left, Expression right) {
	Ymir::log ("Solve inside var : ", tmps, "|", left, "|", right);
	if (auto tvar = left-> to<ITypedVar> ())
	    return this-> solveInside (tmps, tvar, right);
	else if (auto of = left-> to <IOfVar> ())
	    return this-> solveInside (tmps, of, right);
	else if (auto vvar = left-> to <IVariadicVar> ())
	    return this-> solveInside (tmps, vvar, right);
	else if (auto type = right-> to<IType> ()) 
	    return this-> solveInside (left, type);
	else if (auto var = right-> to <IVar> ()) 
	    return this-> solveInside (left, var);
	else if (auto func = right-> to <IFuncPtr> ()) 
	    return this-> solveInside (left, func);
	else return TemplateSolution (0, false);
    }

    TemplateSolution TemplateSolver::solveInside (Var left, Type right) {
	Ymir::log ("Solve inside type : ", left, "|", right);
	auto type = right-> info-> type ()-> cloneOnExit ();
	if (auto fn = type-> to <IFunctionInfo> ()) {
	    if (fn-> isLambda ()) { 
		type = fn-> toPtr (left-> token);
		if (type == NULL) return TemplateSolution (0, false);
	    }
	}
	auto clo = new (Z0) IType (right-> token,  type);
	map<string, Expression> value =  {{left-> token.getStr (), clo}};
	return TemplateSolution (__VAR__, true, type, value);
    }
    
    TemplateSolution TemplateSolver::solveInside (Var left, Var right) {
	Ymir::log ("Solve inside var : ", left, "|", right);
	auto type = right-> info-> type ();
	if (auto ty = type-> to <IStructCstInfo> ()) {
	    vector <Expression> ignore;
	    auto clo = new (Z0) IType (right-> token, ty-> TempOp (ignore));
	    map<string, Expression> value =  {{left-> token.getStr (), clo}};
	    return TemplateSolution (__VAR__, true, type, value);
	} else if (auto ty = type-> to <IAggregateCstInfo> ()) {
	    vector <Expression> ignore;
	    auto clo = new (Z0) IType (right-> token, ty-> TempOp (ignore));
	    map<string, Expression> value =  {{left-> token.getStr (), clo}};
	    return TemplateSolution (__VAR__, true, type, value);
	}
	return TemplateSolution (0, false);
    }

    TemplateSolution TemplateSolver::solveInside (Var left, FuncPtr right) {
	Ymir::log ("Solve inside fn : ", left, "|", right);
	auto type = right-> info-> type ();
	auto clo = new (Z0) IType (right-> token, type-> cloneOnExit ());
	map<string, Expression> value =  {{left-> token.getStr (), clo}};
	return TemplateSolution (__VAR__, true, type, value);
    }
    
    TemplateSolution TemplateSolver::solveInside (const vector <Expression> &tmps, OfVar left, Expression right) {
	Ymir::log ("Solve inside of : ", tmps, "|", left, "|", right);
	InfoType info = NULL;;	
	if (auto all = right-> to <IArrayAlloc> ()) info = all-> info-> type ();
	else if (auto v = right-> to <IVar> ()) {
	    if (v-> info-> type ()-> is <IStructCstInfo> ()) info = v-> info-> type ()-> TempOp ({});
	    else if (v-> info-> type ()-> is <IAggregateCstInfo> ()) info = v-> info-> type ()-> TempOp ({});
	}
	
	if (auto type = right-> to<IType> ()) info = type-> info-> type ();
	if (!info) return TemplateSolution (0, false);

	auto res = this-> solveInside (tmps, left-> getTypeVar (), info);
	if (!res.valid || !res.type-> CompOp (info))
	    return TemplateSolution (0, false);
	else {
	    auto clo = right-> clone ();
	    clo-> info-> type (clo-> info-> type ()-> cloneOnExit ());
	    res.type = info-> cloneOnExit ();
	    map <string, Expression> ret = {{left-> token.getStr (), clo}};
	    if (!merge (res.score, res.elements, ret))
		return TemplateSolution (0, false);
	    return res;
	}	
    }
    
    TemplateSolution TemplateSolver::solveInside (const std::vector<syntax::Expression>  & tmps, syntax::VariadicVar var, syntax::Expression expr) {	
	Ymir::log ("Solve inside variadic ", tmps, "|", var, "|", expr);
	if (var-> isValue ()) {
	    if (!expr-> info-> isImmutable ()) {
		Ymir::Error::notImmutable (expr-> token, expr-> info);
		return TemplateSolution (0, false);
	    }
	    map <string, Expression> ret = {{var-> token.getStr(), expr-> info-> value ()-> toYmir (expr-> info)}};
	    return TemplateSolution {0, true, expr-> info-> type ()-> cloneOnExit (), ret};
	} else {
	    Ymir::Error::assert ("TODO");
	}
	return TemplateSolution (0, false);
    }

    TemplateSolution TemplateSolver::solveInside (const vector <Expression> & tmps, TypedVar left, Expression right) {
	Ymir::log ("Solve inside typed : ", tmps, "|", left, "|", right);
	auto type = right-> info-> type ();
	
	if (!right-> info-> isImmutable ()) {
	    Ymir::Error::notImmutable (right-> token, right-> info);
	    return TemplateSolution (0, false);
	}
	
	TemplateSolution res (0, false);
	if (auto var = left-> typeExp ()-> to <IVar> ()) {
	    res = this-> solveInside (tmps, var, type);
	    if (!res.valid)
		return TemplateSolution (0, false);
	    
	    if (!type-> isSame (res.type)) 
		return TemplateSolution (0, false);
	} else {
	    res = this-> solveInside (tmps, left-> typeExp ()-> to <IFuncPtr> (), type);
	    if (!res.valid)
		return TemplateSolution (0, false);
	}
	       
	map <string, Expression> ret = {{left-> token.getStr (), right-> info-> value ()-> toYmir (right-> info)}};
	if (!merge (res.score, res.elements, ret))
	    return TemplateSolution (0, false);
	return res;
    }
    
    TemplateSolution TemplateSolver::solveInside (const vector <Expression> & tmps, Expression left , Expression right) {
	Ymir::log ("Solve inside expr 3 : ", tmps, "|", left, "|", right);
	if (!right-> info-> isImmutable ()) {
	    Ymir::Error::notImmutable (right-> token, right-> info);
	    return TemplateSolution (0, false);
	}
	
	auto elem = left-> expression ();
	if (elem == NULL) return TemplateSolution (0, false);
	else if (!elem-> info-> isImmutable ()) {
	    Ymir::Error::notImmutable (right-> token, right-> info);
	    return TemplateSolution (0, false);
	}
	    
	if (!elem-> info-> value ()-> equals (right-> info-> value ()))
	    return TemplateSolution (0, false);
	
	auto it = 0;
	for (auto et : tmps) {
	    if (left == et) break;
	    else it++;
	}
	
	map<string, Expression> elems = {{Ymir::OutBuffer (it).str (), right}};
	return TemplateSolution (__VALUE__, true, elem-> info-> type (), elems);
    }

    TemplateSolution TemplateSolver::solve (const vector <Var> &tmps, const vector <Expression> &params) {
	Ymir::log ("Solve inside all : ", tmps, "|", params);
	vector <Expression> aux;
	for (auto it : tmps)
	    aux.push_back (it);
	return solve (aux, params);
    }

    bool TemplateSolver::isValid (const vector <Expression> &args) {
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

    bool TemplateSolver::isSolved (const vector <Expression> &args, TemplateSolution soluce) {
	if (!soluce.valid) return false;
	return isSolved (args, soluce.elements);
    }

    bool TemplateSolver::isSolved (const vector <Expression> &args, map <string, Expression> &types) {
	ulong nb = 0;
	for (auto it : args) {
	    if (auto type = it-> to <ITypedVar> ()) {
		auto elem = types.find (type-> token.getStr ());
		if (elem != types.end () && elem-> second == NULL)
		    return false;
		else if (elem == types.end ()) return false;
	    } else if (it-> is <IVar> ()) {		
		auto elem = types.find (it-> token.getStr ());
		if (elem != types.end () && elem-> second == NULL)
		    return false;
		else if (elem == types.end ()) return false;
	    } else {
		Ymir::OutBuffer buf;
		buf.write (nb);
		if (types.find (buf.str ()) == types.end ()) return false;
	    }
	    nb ++;
	}
	return true;
    }

    vector <Expression> TemplateSolver::solved (const vector <Expression> &args, TemplateSolution soluce) {
	std::vector <Expression> rets;
	ulong nb = 0;
	for (auto it : args) {
	    if (auto v = it-> to<IVar> ()) {
		auto elem = soluce.elements.find (v-> token.getStr ());
		if (elem != soluce.elements.end ()) {
		    auto val = elem-> second-> templateExpReplace ({});
		    if (val && !val-> info && !val-> is<IParamList> ()) val = val-> expression ();
		    rets.push_back (val);		    
		}
	    } else {
		Ymir::OutBuffer buf;
		buf.write (nb);
		if (soluce.elements.find (buf.str ()) != soluce.elements.end ()) {
		    if (it-> info) 
			rets.push_back (it-> info-> value ()-> toYmir (it-> info)-> expression ());
		    else
			rets.push_back (it-> templateExpReplace (soluce.elements)-> expression ());
		}
	    }
	    nb ++;
	}
	return rets;
    }

    
    vector <Expression> TemplateSolver::solved (const vector <Expression> &args, map<string, Expression>& soluce) {
	std::vector <Expression> rets;
	ulong nb = 0;
	for (auto it : args) {
	    if (auto v = it-> to<IVar> ()) {
		auto elem = soluce.find (v-> token.getStr ());
		if (elem != soluce.end ()) {
		    auto val = elem-> second-> templateExpReplace ({});
		    if (val && !val-> info && !val-> is <IParamList> ()) val = val-> expression ();
		    rets.push_back (val);		    
		}
	    } else {
		Ymir::OutBuffer buf;
		buf.write (nb);
		if (soluce.find (buf.str ()) != soluce.end ()) {
		    if (it-> info) 
			rets.push_back (it-> info-> value ()-> toYmir (it-> info)-> expression ());
		    else
			rets.push_back (it-> templateExpReplace (soluce)-> expression ());
		}
	    }
	    nb ++;
	}
	return rets;
    }

    vector <Expression> TemplateSolver::unSolved (const vector <Expression> &args, TemplateSolution soluce) {
	std::vector <Expression> rets;
	ulong nb = 0;
	for (auto it : args) {
	    if (auto v = it-> to<IVar> ()) {
		auto elem = soluce.elements.find (v-> token.getStr ());
		if (elem == soluce.elements.end ())
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
