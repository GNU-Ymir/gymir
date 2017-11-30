#include <ymir/ast/_.hh>
#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/pack/_.hh>
#include <ymir/semantic/value/_.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/utils/Array.hh>


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
	auto call = new IPar (this-> token, this-> token, var, finalParams, true);

	auto res = call-> expression ();
	auto errors = Ymir::Error::caught ();
	Ymir::Error::activeError (true);
	
	if (errors.size () != 0) return NULL;
	else return res;	
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
    
    
    Expression IArrayAlloc::expression () {
	auto aux = new IArrayAlloc (this-> token, NULL, this-> size-> expression ());
	if (auto fn = this-> type-> to<IFuncPtr> ()) aux-> type = fn-> expression ();
	else if (auto type = this-> type-> to<IVar> ()) aux-> type = type-> asType ();
	else Ymir::Error::useAsType (this-> type-> token);

	// if (auto type = aux-> type-> info-> type-> to<IStructCstInfo> ()) {
	//     Ymir::Error::assert ("TODO");
	// }

	auto ul = new ISymbol (this-> token, new IFixedInfo (true, FixedConst::ULONG));
	auto cmp = aux-> size-> info-> type-> CompOp (ul-> type);
	if (cmp == NULL) {
	    Ymir::Error::assert ("TODO");
	}
	aux-> cster = cmp;
	aux-> info = new ISymbol (this-> token, new IArrayInfo (false, aux-> type-> info-> type-> clone ()));
	return aux;
    }
    
    Expression IBinary::expression () {
	if (this-> token == Token::EQUAL) {
	    return affect ();
	} else if (canFind (std::vector <std::string> {Token::DIV_AFF, Token::AND_AFF, Token::PIPE_EQUAL,
			Token::MINUS_AFF, Token::PLUS_AFF, Token::LEFTD_AFF,
			Token::RIGHTD_AFF, Token::STAR_EQUAL,
			Token::PERCENT_EQUAL, Token::XOR_EQUAL,
			Token::DXOR_EQUAL, Token::TILDE_EQUAL
			}, this-> token.getStr ())) {
	    return reaff ();
	} else return normal ();
    }

    bool IBinary::simpleVerif (Binary aux) {
	if (aux-> left == NULL || aux-> right == NULL) return true;
	else if (aux-> left-> is<IType> ()) {
	    Ymir::Error::useAsVar (aux-> left-> token, aux-> left-> info);
	    return true;
	} else if (aux-> right-> is<IType> ()) {
	    Ymir::Error::useAsVar (aux-> right-> token, aux-> right-> info);
	    return true;
	} else if (aux-> right-> info == NULL) {
	    Ymir::Error::undefinedOp (this-> token, aux-> left-> info, new IVoidInfo ());
	    return true;
	} else if (aux-> right-> info-> isType ()) {
	    Ymir::Error::useAsVar (aux-> right-> token, aux-> right-> info);
	    return true;
	} else if (aux-> left-> info == NULL) {
	    Ymir::Error::undefVar (aux-> left-> token, Table::instance ().getAlike (aux-> left-> token.getStr ()));
	    return true;
	} else if (aux-> left-> info-> isType ()) {
	    Ymir::Error::useAsVar (aux-> left-> token, aux-> left-> info);
	    return true;
	} else if (aux-> right-> info-> type-> is<IUndefInfo> ()) {
	    Ymir::Error::uninitVar (aux-> right-> token);
	    return true;
	}
	return false;
    }
    
    Expression IBinary::affect () {
	auto aux = new IBinary (this-> token, this-> left-> expression (), this-> right-> expression ());	
	if (simpleVerif (aux)) return NULL;
	
	auto type = aux-> left-> info-> type-> BinaryOp (this-> token, aux-> right);
	if (type == NULL) {
	    if (aux-> left-> info-> type-> is<IUndefInfo> ()) {
		type = aux-> right-> info-> type-> BinaryOpRight (this-> token, aux-> left);
		if (type == NULL) {
		    Ymir::Error::undefinedOp (this-> token, aux-> left-> info, aux-> right-> info);
		    return NULL;
		}

		aux-> left-> info-> type = type;
		aux-> left-> info-> type-> isConst () = false;
		aux-> isRight = true;
	    } else {
		Ymir::Error::undefinedOp (this-> token, aux-> left-> info, aux-> right-> info);
		return NULL;
	    }
	}

	aux-> info = new ISymbol (aux-> token, type);
	Table::instance ().retInfo ().changed () = true;
	return aux;	
    }

    Expression IBinary::reaff () {
	auto aux = new IBinary (this-> token, this-> left-> expression (), this-> right-> expression ());

	if (simpleVerif (aux)) return NULL;
	else if (aux-> left-> info-> type-> is<IUndefInfo> ()) {
	    Ymir::Error::uninitVar (aux-> left-> token);
	    return NULL;
	}

	auto type = aux-> left-> info-> type-> BinaryOp (this-> token, aux-> right);
	if (type == NULL) {
	    auto call = findOpAssign (aux);
	    if (!call) {
		Ymir::Error::undefinedOp (this-> token, aux-> left-> info, aux-> right-> info);
		return NULL;
	    }
	    return call;	    
	}

	aux-> info = new ISymbol (aux-> token, type);
	Table::instance ().retInfo ().changed () = true;
	return aux;	
    }

    Expression IBinary::normal () {	
	if (!this-> info) {
	    auto aux = new IBinary (this-> token, this-> left-> expression (), this-> right-> expression ());
	    if (simpleVerif (aux)) return NULL;
	    else if (aux-> left-> info-> type-> is<IUndefInfo> ()) {
		Ymir::Error::uninitVar (aux-> left-> token);
		return NULL;
	    }

	    auto type = aux-> left-> info-> type-> BinaryOp (this-> token, aux-> right);
	    if (type == NULL) {
		type = aux-> right-> info-> type-> BinaryOpRight (this-> token, aux-> left);
		if (type == NULL) {
		    auto call = findOpBinary (aux);
		    if (!call) {
			Ymir::Error::undefinedOp (this-> token, aux-> left-> info, aux-> right-> info);
			return NULL;
		    }
		    return call;
		}
		aux-> isRight = true;		
	    }

	    aux-> info = new ISymbol (aux-> token, type);
	    return aux;
	} else {
	    auto aux = new IBinary (this-> token, NULL, NULL);
	    aux-> info = this-> info;
	    return aux;
	}	
    }

    Expression IBinary::findOpAssign (Binary aux) {
	Ymir::Error::activeError (false);
	Word word (this-> token.getLocus (), Keys::OPASSIGN);
	auto var = new IVar (word, {new IString (this-> token, this-> token.getStr ())});
	auto params = new IParamList (this-> token, {aux-> left, aux-> right});
	auto call = new IPar (this-> token, this-> token, var, params, false);
	
	auto res = call-> expression ();
	auto errors = Ymir::Error::caught ();
	Ymir::Error::activeError (true);
	
	if (errors.size () != 0) return NULL;
	else return res;
    }

    Expression IBinary::findOpBinary (Binary aux) {
	if (isTest (this-> token)) return findOpTest (aux);
	else if (isEq (this-> token)) return findOpEqual (aux);
	Ymir::Error::activeError (false);

	Word word (this-> token.getLocus (), Keys::OPBINARY);
	auto var = new IVar (word, {new IString (this-> token, this-> token.getStr ())});
	auto params = new IParamList (this-> token, {aux-> left, aux-> right});

	auto call = new IPar (this-> token, this-> token, var, params, false);
	auto res = call-> expression ();	
	auto errors = Ymir::Error::caught ();
	Ymir::Error::activeError (true);
	
	if (errors.size () != 0) {
	    Ymir::Error::activeError (false);
	    word = Word (this-> token.getLocus (), Keys::OPBINARYR);
	    var = new IVar (word, {new IString (this-> token, this-> token.getStr ())});
	    params = new IParamList (this-> token, {aux-> right, aux-> left});
	    call = new IPar (this-> token, this-> token, var, params, false);
	    res = call-> expression ();
	    errors = Ymir::Error::caught ();
	    Ymir::Error::activeError (true);
	    if (errors.size () != 0) return NULL;
	}	
	
	return res;
    }

    Expression IBinary::findOpTest (Binary aux) {
	Ymir::Error::assert ("TODO");
    }
    
    Expression IBinary::findOpEqual (Binary aux) {
	Ymir::Error::assert ("TODO");
    }
    
    bool IBinary::isTest (Word elem) {
	return canFind (std::vector <std::string> {Token::INF, Token::SUP, Token::INF_EQUAL, Token::SUP_EQUAL, Token::NOT_INF, Token::NOT_SUP, Token::NOT_INF_EQUAL, Token::NOT_SUP_EQUAL}, elem.getStr ());
    }

    bool IBinary::isEq (Word elem) {
	return elem == Token::DEQUAL || elem == Token::NOT_EQUAL;
    }

    Expression IFixed::expression () {
	auto aux = new IFixed (this-> token, this-> type);
	aux-> info = new ISymbol (this-> token, new IFixedInfo (true, this-> type));
	aux-> info-> value = new IFixedValue (this-> token.getStr ());
	return aux;
    }
    
    Expression IChar::expression () {
	auto aux = new IChar (this-> token, this-> code);
	aux-> info = new ISymbol (this-> token, new ICharInfo (true));
	aux-> info-> value = new ICharValue (this-> code);
	return aux;
    }

    Expression IFloat::expression () {
	auto aux = new IFloat (this-> token, this-> suite);
	aux-> info = new ISymbol (this-> token, new IFloatInfo (true, this-> _type));
	//TODO
	return aux;
    }

    Expression IString::expression () {
	auto aux = new IString (this-> token, this-> content);
	aux-> info = new ISymbol (this-> token, new IStringInfo (true));
	//TODO
	return aux;
    }
    
}
