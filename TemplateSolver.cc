#include <ymir/semantic/pack/TemplateSolver.hh>
#include <ymir/ast/TypedVar.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/semantic/value/_.hh>

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
			    if (auto ref = ltype-> info-> type-> to <IRefInfo> ()) {
				if (!rtype-> info-> type-> isSame (ref-> content ()))
				    return false;
				else {
				    left [it.first] = inside-> second;
				}
			    } else if (auto ref = rtype-> info-> type-> to <IRefInfo> ()) {
				if (!ltype-> info-> type-> isSame (ref-> content ()))
				    return false;
				else {
				    left [it.first] = it.second;
				}
			    } else {		
				return false;
			    }
			}
		    }
		} else if (!ltype-> info-> type-> is <IUndefInfo> () &&
			   !rtype-> info-> type-> is <IUndefInfo> () &&
			   !ltype-> info-> type-> isSame (rtype-> info-> type)) {
		    if (auto ref = ltype-> info-> type-> to <IRefInfo> ()) {
			if (!rtype-> info-> type-> isSame (ref-> content ()))
			    return false;
			else {
			    left [it.first] = inside-> second;
			}
		    } else if (auto ref = rtype-> info-> type-> to <IRefInfo> ()) {
			if (!ltype-> info-> type-> isSame (ref-> content ()))
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
			    if (auto ref = ltype-> info-> type-> to <IRefInfo> ()) {
				if (!rtype-> info-> type-> isSame (ref-> content ()))
				    return false;
				else {
				    left [it.first] = inside-> second;
				}
			    } else if (auto ref = rtype-> info-> type-> to <IRefInfo> ()) {
				if (!ltype-> info-> type-> isSame (ref-> content ()))
				    return false;
				else {
				    left [it.first] = it.second;
				}
			    } else return false;
			}
		    }
		} else if (!ltype-> info-> type-> is <IUndefInfo> () &&
			   !rtype-> info-> type-> is <IUndefInfo> () &&
			   !ltype-> info-> type-> isSame (rtype-> info-> type)) {
		    if (auto ref = ltype-> info-> type-> to <IRefInfo> ()) {
			if (!rtype-> info-> type-> isSame (ref-> content ()))
			    return false;
			else {
			    left [it.first] = inside-> second;
			}
		    } else if (auto ref = rtype-> info-> type-> to <IRefInfo> ()) {
			if (!ltype-> info-> type-> isSame (ref-> content ()))
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
	if (auto t = type-> to <IRefInfo> ()) type = t-> content ();
	//if (auto t = type-> to <IEnumInfo> ()) type = t-> getContent ();
	if (auto tvar = param-> to <ITypedVar> ()) {
	    if (tvar-> typeExp ()) {
		if (auto all = tvar-> typeExp ()-> to<IArrayAlloc> ()) {
		    return solve (tmps, all, type);
		} 
		return solve (tmps, tvar-> typeExp (), type);
	    }

	    bool isConst = false;				
	    auto typeVar = tvar-> typeVar ();
	    while (typeVar-> token == Keys::CONST) {
		isConst = true;
		typeVar = typeVar-> getTemplates () [0]-> to <IVar> ();
	    }
	    
	    if (auto arr = typeVar-> to <IArrayVar> ()) {
		return solve (tmps, arr, type, isConst);
	    } else if (auto all = typeVar-> to <IArrayAlloc> ()) {
		return solve (tmps, all, type, isConst);
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
		soluce.type = var-> info-> type;
		soluce.type-> isConst (isConst);
		soluce.score += __VAR__;
		return soluce;
	    }
	}
	return TemplateSolution (0, false);
    }
    
    TemplateSolution TemplateSolver::solveInside (const vector <Expression> &tmps, Var param, InfoType type) {
	//if (auto t = type-> to <IRefInfo> ()) type = t-> content ();
	// if (auto t = type-> to <IEnumInfo> ()) type = t-> getContent ();
	bool isConst = false;	    
	while (param && param-> token == Keys::CONST) {
	    isConst = true;
	    param = param-> getTemplates () [0]-> to <IVar> ();
	}
	if (param == NULL) return TemplateSolution (0, false);

	if (auto arr = param-> to <IArrayVar> ()) {
	    return solve (tmps, arr, type, isConst);
	} else if (auto all = param-> to <IArrayAlloc> ()) {
	    return solve (tmps, all, type, isConst);
	} else if (auto ty = param-> to <IType> ()) {
	    TemplateSolution soluce (0, true);
	    soluce.type = ty-> info-> type-> clone ();
	    soluce.type-> isConst (isConst);
	    if (!ty-> info-> type-> isSame (type) && !type-> is<IUndefInfo> ())
		return TemplateSolution (0, false);
	    soluce.score += __VAR__;
	    return soluce;
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

	    if (!var-> info-> type-> isSame (type) && !type-> is<IUndefInfo> ())
		return TemplateSolution (0, false);
	    soluce.type = var-> info-> type;
	    soluce.type-> isConst (isConst);
	    soluce.score += __VAR__;
	    return soluce;
	}    
    }
	
    TemplateSolution TemplateSolver::solveInside (const vector <Expression> &tmps, Var var, const vector <InfoType> &type) {
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

    TemplateSolution TemplateSolver::solve (const vector <Expression> &tmps, ArrayVar param, InfoType type, bool isConst) {
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
		paramSize-> info = new (Z0) ISymbol (paramSize-> token, size);
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
	    paramSize-> info = new (Z0) ISymbol (paramSize-> token, size);
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
	if (auto func = elem-> to <IFuncPtr> ()) {
	    return solveInside (tmps, func, type);
	}
	return TemplateSolution (0, false);
    }

    TemplateSolution TemplateSolver::solveInside (const vector <Expression> & tmps, FuncPtr func, InfoType type) {
	vector <Var> types;
	TemplateSolution soluce (0, true);
	// if (type-> nbTemplates () != func-> getParams ().size () + 1)
	//     return TemplateSolution (0, false);

	ulong nb = 0;
	for (auto it : Ymir::r (0, func-> getParams ().size ())) {
	    auto var = func-> getParams () [it];
	    if (!type-> getTemplate (it)) return TemplateSolution (0, false);
	    auto typeTemplates = type-> getTemplate (it, func-> getParams ().size () - (it + 1));
	    nb += typeTemplates.size ();
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
	    soluce.type = var-> info-> type;
	    soluce.type-> isConst (false);
	    soluce.score += __VAR__;
	    return soluce;
	}
	return TemplateSolution (0, false);
    }

    TemplateSolution TemplateSolver::solve (Var elem, Var param, InfoType type, bool isConst) {
	if (elem-> is<ITypedVar> ()) return TemplateSolution (0, false);
	else if (elem-> is<IArrayVar> ()) return TemplateSolution (0, false);
	else if (elem-> is<IArrayAlloc> ()) return TemplateSolution (0, false);

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
	if (elem-> is<ITypedVar> ()) return TemplateSolution (0, false);
	else if (elem-> is<IArrayVar> ()) return TemplateSolution (0, false);
	else if (elem-> is<IArrayAlloc> ()) return TemplateSolution (0, false);
	else if (type-> is<IFunctionInfo> ()) return TemplateSolution (0, false);	
	// TODO Object, Struct

	if (elem-> token == Keys::CONST) {
	    return solve (elem-> getTemplates () [0]-> to <IVar> (), param, type, true);	    
	} else if (elem-> getDeco () == Keys::CONST)
	    isConst = true;
	
	auto type_ = type-> cloneOnExit ();
	type_-> isConst (isConst);
	
	map <string, Expression> ret;	
	ret [elem-> token.getStr ()] = new (Z0)  IType (param-> token, type_);
	return TemplateSolution (__VAR__, true, type_, ret);
    }

    TemplateSolution TemplateSolver::solve (const vector <Expression> &tmps, OfVar elem, Var param, InfoType type, bool isConst) {
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
	    map<string, Expression> ret = {{elem-> token.getStr (), new (Z0)  IType (typeVar-> token, type_)}};
	    if (!merge (res.score, res.elements, ret))
		return TemplateSolution (0, false);
	    res.score += __VAR__;
	    return res;
	}
	
    }

    Expression getAndRemove (std::string elem, std::map <std::string, Expression> & elements) {
	auto it = elements.find (elem);
	if (it == elements.end ()) return NULL;
	elements.erase (elem);
	return it-> second;
    }
    
    TemplateSolution TemplateSolver::solveVariadic (const std::vector <Expression> tmps, Expression last, std::vector <Expression> params) {
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
	if (params.size () == 1) {
	    auto var = new (Z0) IVar (last-> token);
	    return solveInside (tmps, var, params [0]);
	} else {
	    TemplateSolution soluce (0, true);
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
	TemplateSolution soluce (0, true);
	if (tmps.size () != 0 && tmps [tmps.size () - 1]-> is <IVariadicVar> ()) {
	    return solveVariadic (tmps, params);
	} else if (tmps.size () < params.size ()) {
	    return TemplateSolution (0, false);
	}
	
	for (auto it : Ymir::r (0, params.size ())) {
	    TemplateSolution res (0, true);
	    if (auto v = tmps [it]-> to<IVar> ()) {
		res = this-> solveInside (tmps, v, params [it]);
	    } else {
		res = this-> solveInside (tmps, tmps [it], params [it]);
	    }
	    
	    if (!res.valid || !merge (soluce.score, soluce.elements, res))
		return TemplateSolution (0, false);
	}
	return soluce;	
    }
    
    TemplateSolution TemplateSolver::solveInside (const vector <Expression> &tmps, Var left, Expression right) {
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
	auto type = right-> info-> type;
	auto clo = new (Z0) IType (right-> token,  type-> cloneOnExit ());
	map<string, Expression> value =  {{left-> token.getStr (), clo}};
	return TemplateSolution (__VAR__, true, type, value);
    }
    
    TemplateSolution TemplateSolver::solveInside (Var left, Var right) {
	auto type = right-> info-> type;
	if (auto ty = type-> to <IStructCstInfo> ()) {
	    vector <Expression> ignore;
	    auto clo = new (Z0) IType (right-> token, ty-> TempOp (ignore));
	    map<string, Expression> value =  {{left-> token.getStr (), clo}};
	    return TemplateSolution (__VAR__, true, type, value);
	}
	return TemplateSolution (0, false);
    }

    TemplateSolution TemplateSolver::solveInside (Var left, FuncPtr right) {
	auto type = right-> info-> type;
	auto clo = new (Z0) IType (right-> token, type-> cloneOnExit ());
	map<string, Expression> value =  {{left-> token.getStr (), clo}};
	return TemplateSolution (__VAR__, true, type, value);
    }
    
    TemplateSolution TemplateSolver::solveInside (const vector <Expression> &tmps, OfVar left, Expression right) {
	InfoType info = NULL;;	
	if (auto all = right-> to <IArrayAlloc> ()) info = all-> info-> type;
	else if (auto v = right-> to <IVar> ()) {
	    if (v-> info-> type-> is <IStructCstInfo> ()) info = v-> info-> type-> TempOp ({});
	}
	
	if (auto type = right-> to<IType> ()) info = type-> info-> type;
	if (!info) return TemplateSolution (0, false);

	auto res = this-> solveInside (tmps, left-> typeVar (), info);
	if (!res.valid || !res.type-> CompOp (info))
	    return TemplateSolution (0, false);
	else {
	    auto clo = right-> clone ();
	    clo-> info-> type = clo-> info-> type-> cloneOnExit ();
	    res.type = info-> cloneOnExit ();
	    map <string, Expression> ret = {{left-> token.getStr (), clo}};
	    if (!merge (res.score, res.elements, ret))
		return TemplateSolution (0, false);
	    return res;
	}	
    }
    
    TemplateSolution TemplateSolver::solveInside (const std::vector<syntax::Expression>  &, syntax::VariadicVar, syntax::Expression) {
	Ymir::Error::assert ("TODO");
	return TemplateSolution (0, false);
    }

    TemplateSolution TemplateSolver::solveInside (const vector <Expression> & tmps, TypedVar left, Expression right) {
	auto type = right-> info-> type;
	
	if (!right-> info-> isImmutable ()) {
	    Ymir::Error::notImmutable (right-> token, right-> info);
	    return TemplateSolution (0, false);
	}
	
	TemplateSolution res (0, false);
	if (left-> typeVar ()) {
	    res = this-> solveInside (tmps, left-> typeVar (), type);
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
	return TemplateSolution (__VAR__, true, elem-> info-> type, elems);
    }

    TemplateSolution TemplateSolver::solve (const vector <Var> &tmps, const vector <Expression> &params) {
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

    vector <Expression> TemplateSolver::solved (const vector <Expression> &args, TemplateSolution soluce) {
	std::vector <Expression> rets;
	ulong nb = 0;
	for (auto it : args) {
	    if (auto v = it-> to<IVar> ()) {
		auto elem = soluce.elements.find (v-> token.getStr ());
		if (elem != soluce.elements.end ()) {
		    auto val = elem-> second-> templateExpReplace ({});
		    if (val && val-> info) val-> info-> isConst (false);
		    rets.push_back (val);		    
		}
	    } else {
		Ymir::OutBuffer buf;
		buf.write (nb);
		if (soluce.elements.find (buf.str ()) != soluce.elements.end ()) {
		    if (it-> info) 
			rets.push_back (it-> info-> value ()-> toYmir (it-> info));
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
		    if (val && val-> info) val-> info-> isConst (false);
		    rets.push_back (val);		    
		}
	    } else {
		Ymir::OutBuffer buf;
		buf.write (nb);
		if (soluce.find (buf.str ()) != soluce.end ()) {
		    if (it-> info) 
			rets.push_back (it-> info-> value ()-> toYmir (it-> info));
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
