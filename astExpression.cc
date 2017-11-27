#include <ymir/ast/_.hh>
#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/pack/_.hh>
#include <ymir/syntax/Keys.hh>

namespace syntax {
    using namespace semantic;
    
    Expression IAccess::expression () {
	auto aux = new IAccess (this-> token, this-> end);
	aux-> params = (ParamList) this-> params-> expression ();
	aux-> left = this-> left-> expression ();
	if (aux-> left-> is<IType> ())
	    Ymir::Error::undefVar (aux-> left-> token,
				   Table::instance ().getAlike (aux-> left-> token.getStr ())
	    );

	else if (aux-> left-> info-> type-> is <IUndefInfo> ())
	    Ymir::Error::uninitVar (aux-> left-> token);
	else if (aux-> left-> info-> isType ())
	    Ymir::Error::useAsVar (aux-> left-> token, aux-> left-> info);

	auto type = aux-> left-> info-> type-> AccessOp (aux-> left-> token,
							 aux-> params);
	if (type == NULL) {
	    auto call = findOpAccess ();
	    if (call == NULL) {
		Ymir::Error::undefinedOp (this-> token, this-> end,
					  aux-> left-> info, aux-> params);
	    } else {
		return call;
	    }
	}
	aux-> info = new ISymbol (this-> token, type);
	return aux;
    }


    Expression IAccess::findOpAccess () {
	Ymir::Error::activeError (false);
	Word word (this-> token.getLocus (), Keys::OPACCESS);
	auto var = new IVar (word);
	std::vector <Expression> params = {this-> left};
	params.insert (params.begin (), this-> params-> getParams ().begin (),
		       this-> params-> getParams ().end ());
	auto finalParams = new IParamList (this-> token, params);
	auto call = IPar (this-> token, this-> token, var, finalParams, true);

	auto errors = Ymir::Error::caught ();
	Ymir::Error::activeError (true);
	
	if (errors.size () != 0) return NULL;
	else return call.expression ();	
    }

    
    
    Expression IVar::expression () {
	if (this-> info && this-> info-> isImmutable ()) {
	    return this;	    
	} else if (!this-> isType ()) {
	    auto aux = new IVar (this-> token);
	    aux-> info = Table::instance ().get (this-> token.getStr ());
	    if (aux-> info == NULL) {
		Ymir::Error::assert ("TODO, gerer l'erreur");
	    }

	    if (this-> templates.size () != 0) {
		if (!this-> inside-> is<IPar> () && !this-> inside-> is<IDot> ()) {
		    Ymir::Error::assert ("TODO");
		} else if (auto dt = this-> inside-> to<IDot> ()) {
		    Ymir::Error::assert ("TODO");
		}
		
		std::vector <Expression> tmps;
		for (auto it : this-> templates) {
		    tmps.push_back (it-> expression ());
		}

		auto type = aux-> info-> type-> TempOp (tmps);
		if (type == NULL) {
		    Ymir::Error::assert ("TODO, gerer l'erreur");
		}
		aux-> templates = tmps;
		aux-> info = new ISymbol (aux-> info-> sym, type);
	    }
	    return aux;
	} else return asType ();	
    }

    Var IVar::var () {
	if (this-> info && this-> info-> isImmutable ()) {
	    return this;	    
	} else if (!this-> isType ()) {
	    auto aux = new IVar (this-> token);
	    aux-> info = Table::instance ().get (this-> token.getStr ());
	    if (aux-> info == NULL) {
		Ymir::Error::assert ("TODO, gerer l'erreur");
	    }

	    if (this-> templates.size () != 0) {
		std::vector <Expression> tmps;
		for (auto it : this-> templates) {
		    tmps.push_back (it-> expression ());
		}

		auto type = aux-> info-> type-> TempOp (tmps);
		if (type == NULL) {
		    Ymir::Error::assert ("TODO, gerer l'erreur");
		}
		aux-> templates = tmps;
		aux-> info = new ISymbol (aux-> info-> sym, type);
	    }
	    return aux;
	} else return asType ();
    }
    
    TypedVar IVar::setType (Symbol info) {
	if (this-> deco == Keys::REF) {
	    auto type = new IType (info-> sym, info-> type-> cloneOnExit ());
	    return new ITypedVar (this-> token, type);
	} else {
	    auto type = new IType (info-> sym, info-> type-> cloneOnExit ());
	    return new ITypedVar (this-> token, type, this-> deco);
	}
    }

    TypedVar IVar::setType (InfoType info) {
	if (this-> deco == Keys::REF) {
	    auto type = new IType (this-> token, info-> cloneOnExit ());
	    return new ITypedVar (this-> token, type);
	} else {
	    auto type = new IType (this-> token, info-> cloneOnExit ());
	    return new ITypedVar (this-> token, type, this-> deco);
	}
    }

    Type IVar::asType () {
	std::vector <Expression> tmps;
	for (auto it : this-> templates)
	    tmps.push_back (it-> expression ());
	
	if (!IInfoType::exists (this-> token.getStr ())) {	    
	    Ymir::Error::assert ("TODO");
	} else {
	    auto t_info = IInfoType::factory (this-> token, tmps);
	    if (this-> deco == Keys::REF)
		t_info = new IRefInfo (t_info);
	    else if (this-> deco == Keys::CONST) t_info-> isConst () = true;
	    else t_info-> isConst () = false;
	    return new IType (this-> token, t_info);
	}
    }

    bool IVar::isType () {
	if (IInfoType::exists (this-> token.getStr ())) return true;
	else {
	    auto info = Table::instance ().get (this-> token.getStr ());
	    if (info) Ymir::Error::assert ("TODO");
	}
	return false;	
    }

    
    semantic::InfoType ITypedVar::getType () {
	if (this-> type) {
	    auto type = this-> type-> asType ();
	    if (this-> deco == Keys::REF && !type-> info-> type-> is <IRefInfo> ()) {
		if (type-> info-> type-> is<IEnumInfo> ()) 
		    Ymir::Error::assert ("TODO");
		return new IRefInfo (type-> info-> type);
	    } else if (this-> deco == Keys::CONST) {
		type-> info-> type-> isConst () = true;
	    } else type-> info-> type-> isConst () = false;
	    return type-> info-> type;	    
	} else {
	    this-> expType = this-> expType-> expression ();
	    auto type = this-> expType;
	    if (this-> deco == Keys::REF && !type-> info-> type-> is <IRefInfo> ()) {
		if (type-> info-> type-> is<IEnumInfo> ()) 
		    Ymir::Error::assert ("TODO");
		return new IRefInfo (type-> info-> type);
	    } else if (this-> deco == Keys::CONST) {
		type-> info-> type-> isConst () = true;
	    } else type-> info-> type-> isConst () = false;
	    return type-> info-> type;	    
	}
    }

    Expression ITypedVar::expression () {
	TypedVar aux;
	if (this-> type) {
	    aux = new ITypedVar (this-> token, this-> type-> asType ());
	} else {
	    auto ptr = this-> expType-> expression ()-> to<IFuncPtr> ();
	    if (ptr) {
		aux = new ITypedVar (this-> token, new IType (ptr-> token, ptr-> info-> type));
	    } else Ymir::Error::assert ("????!!!!");	
	}
	
	if (this-> deco == Keys::REF) {
	    aux-> info = new ISymbol (this-> token, new IRefInfo (aux-> type-> info-> type));
	} else {
	    aux -> info = new ISymbol (this-> token, aux-> type-> info-> type);
	    aux-> info-> type-> isConst () = (this-> deco == Keys::CONST);
	}
	Table::instance ().insert (aux-> info);
	return aux;    
    }
    
    Var ITypedVar::var () {
	return (Var) this-> expression ();
    }
    
    

    

    

}
