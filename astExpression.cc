#include <ymir/ast/_.hh>
#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/object/AggregateInfo.hh>
#include <ymir/semantic/pack/_.hh>
#include <ymir/semantic/value/_.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/utils/Array.hh>
#include <ymir/utils/Options.hh>
#include <ymir/semantic/object/MethodInfo.hh>

namespace syntax {
    using namespace semantic;

    const std::string IPragma::COMPILE = "compiles";
    const std::string IPragma::MSG = "msg";
    
    Type IExpression::toType () {
	Expression aux;
	bool derog = false;
	if (auto var = this-> to<IVar> ()) {
	    aux = var-> asType ();
	} else if (auto arr = this-> to <IArrayAlloc> ()) {
	    aux = arr-> staticArray ();
	    derog = true;	    
	} else if (auto tu = this-> to <IConstTuple> ()) {
	    aux = tu-> asType ();	    
	} else aux = this-> expression ();
	if (aux == NULL) return NULL;

	if (auto fn = aux-> to <IFuncPtr> ()) {
	    if (fn-> body () == NULL) derog = true;
	}

	if (!aux-> isType () && !derog) {
	    Ymir::Error::useAsType (aux-> token);
	    return NULL;
	}
	auto type = aux-> info-> type ();
	if (type-> is <IStructCstInfo> () || type-> is <IAggregateCstInfo> ()) {
	    type = type-> TempOp ({});
	    if (!type) return NULL;
	} 
	
	type-> isType (false);
	return new (Z0) IType (this-> token, type);
    }

    bool IExpression::isType () {
	if (this-> is<IType> ()) return true;
	else if (auto all = this-> to <IArrayAlloc> ()) {
	    return all-> info-> type ()-> to <IArrayInfo> ()-> isStatic ();
	} else if (auto tu = this-> to <ITupleInfo> ()) {
	    return tu-> isType ();
	} else if (this-> info && this-> info-> type ()-> isType ()) return true;
	return false;
    }

    bool IExpression::isExpression () {
	auto ist = this-> isType ();
	if (this-> is <IArrayAlloc> ()) return true;
	else return !ist;
    }
    
    Expression IAccess::expression () {
	auto aux = new (Z0)  IAccess (this-> token, this-> end);
	aux-> params = (ParamList) this-> params-> expression ();
	aux-> left = this-> left-> expression ();
	if (aux-> left == NULL) return NULL;
	if (aux-> params == NULL) return NULL;
	if (!aux-> left-> isExpression ()) {
	    Ymir::Error::useAsVar (aux-> left-> token, aux-> left-> info);
	    return NULL;
	}

	else if (aux-> left-> info-> type ()-> is <IUndefInfo> ()) {
	    Ymir::Error::uninitVar (aux-> left-> token, aux-> left-> info-> sym);
	    return NULL;
	}
	
	std::vector <InfoType> treats (aux-> params-> getParams ().size ());
	auto type = aux-> left-> info-> type ()-> AccessOp (aux-> left-> token,
							 aux-> params,
							 treats
	);


	if (type == NULL) {
	    auto call = findOpAccess ();
	    if (call == NULL) {
		Ymir::Error::undefinedOp (this-> token, this-> end,
					  aux-> left-> info, aux-> params);
		return NULL;
	    } else {
		return call;
	    }
	}
	
	aux-> treats = treats;
	aux-> info = new (Z0)  ISymbol (this-> token, aux, type);
	return aux;
    }
       
    Expression IAccess::findOpAccess () {
	Word word (this-> token.getLocus (), Keys::OPACCESS);
	auto var = new (Z0)  IVar (word);
	std::vector <Expression> params = {this-> left};
	params.insert (params.end (), this-> params-> getParams ().begin (),
		       this-> params-> getParams ().end ());

	Word tok {this-> token, Token::LPAR}, tok2 {this-> token, Token::RPAR};
	auto finalParams = new (Z0)  IParamList (this-> token, params);
	auto call = new (Z0)  IPar (tok, tok2, var, finalParams, true);
	
	return call-> expression ();
    }    

    Expression IVar::expression (Symbol sym) {	
	if (sym-> type ()-> is<IAliasCstInfo> ()) {
	    auto aux = sym-> type ()-> to <IAliasCstInfo> ()-> expression ();
	    if (aux == NULL) return NULL;
	    return aux;
	}
	
	auto aux = new (Z0) IVar (this-> token);
	aux-> info = sym;	
	
	if (this-> templates.size () != 0) {
	    if (this-> inside && this-> inside-> is <IDot> ()) {
		if (auto dt = this-> inside-> to<IDot> ()) {
		    if (this == dt-> getLeft ()) {
			auto params = new (Z0)  IParamList (this-> token, {});
			auto call = new (Z0)  IPar (this-> token, this-> token, this, params, true);
			this-> inside = call;
			return call-> expression ();
		    }
		}
	    } else if (this-> templates.size () != 0 && (!this-> inside || !this-> inside-> is <IPar> ())) {
		bool doCall = true;
		if (this-> inside) {
		    if (this-> inside-> is <IStructCst> ()) doCall = false;
		    if (this-> inside-> is <IDColon> ()) doCall = false;
		    if (this-> inside-> is <IDot> ()) doCall = false;
		    if (auto bin = this-> inside-> to <IBinary> ()) {
			if (bin-> token == Token::EQUAL) doCall = false;
		    }
		    if (!sym-> type ()-> is <IFunctionInfo> ()) doCall = false;
		}
		if (doCall) {
		    auto params = new (Z0)  IParamList (this-> token, {});
		    auto call = new (Z0)  IPar (this-> token, this-> token, this, params, true);
		    this-> inside = call;
		    return call-> expression ();
		}
	    } 
		
	    std::vector <Expression> tmps;
	    for (auto it : this-> templates) {
		auto elem = it-> expression ();
		if (elem == NULL) return NULL;
		else if (auto par = elem-> to <IParamList> ())
		    for (auto it : par-> getParams ())
			tmps.push_back (it);
		else tmps.push_back (elem);
	    }

	    auto type = aux-> info-> type ()-> TempOp (tmps);
	    if (type == NULL) {
		Ymir::Error::notATemplate (this-> token, tmps);
		return NULL;
	    }

	    
	    aux-> templates = tmps;
	    aux-> info = new (Z0)  ISymbol (aux-> info-> sym, aux, type);
	}
	    
	return aux;	
    }
    
    Expression IVar::expression () {
	if (this-> info && this-> info-> isImmutable ()) {
	    return this;	    
	} else if (!this-> isTypeV () && this-> deco != Keys::REF) {
	    auto sym = Table::instance ().get (this-> token.getStr ());
	    if (sym == NULL) {
		Ymir::Error::undefVar (this-> token, Table::instance ().getAlike (this-> token.getStr ()));
		return NULL;
	    } else {		
		auto ret = this-> expression (sym);
		if (ret == NULL) return NULL;
		if (Table::instance ().parentFrame (sym) && ret-> is<IVar> ()) {
		    auto var = ret-> to <IVar> ();
		    if (!var-> isType ()) {
			bool found = false;
			for (auto it : Table::instance ().retInfo ().closure)
			    if (it-> token.getStr () == ret-> token.getStr ()) {
				found = true;
				break;
			    }

			if (!found) {
			    Table::instance ().retInfo ().closure.push_back (ret-> to<IVar> ());
			}
			
			ret-> to<IVar> ()-> _fromClosure = true;
			var-> _lastInfo = ret-> info;
			if (Table::instance ().retInfo ().closureMoved ()) {
			    ret-> info = new (Z0) ISymbol (ret-> token, ret, ret-> info-> type ()-> clone ());
			} else {
			    ret-> info = new (Z0) ISymbol (ret-> token, ret, new (Z0) IRefInfo (false, ret-> info-> type ()-> clone ()));
			}
		    }
		}
		return ret;
	    }
	} else return asType ();	
    }

    Var IVar::var () {
	if (this-> info && this-> info-> isImmutable ()) {
	    return this;	    
	} else if (!this-> isTypeV () && this-> deco != Keys::REF) {
	    auto aux = new (Z0)  IVar (this-> token);
	    aux-> info = Table::instance ().get (this-> token.getStr ());
	    if (aux-> info == NULL) {
		Ymir::Error::undefVar (this-> token, Table::instance ().getAlike (this-> token.getStr ()));
		return NULL;
	    }

	    if (this-> templates.size () != 0) {
		std::vector <Expression> tmps;
		for (auto it : this-> templates) {
		    tmps.push_back (it-> expression ());
		}
		auto type = aux-> info-> type ()-> TempOp (tmps);
		if (type == NULL) {
		    Ymir::Error::notATemplate (this-> token, tmps);
		    return NULL;
		}
		aux-> templates = tmps;
		aux-> info = new (Z0)  ISymbol (aux-> info-> sym, aux, type);
	    }
	    return aux;
	} else {
	    return asType ();
	}
    }
    
    TypedVar IVar::setType (Symbol info) {
	auto type = new (Z0)  IType (info-> sym, info-> type ()-> cloneOnExit ());
	return new (Z0)  ITypedVar (this-> token, type, this-> deco);	
    }

    TypedVar IVar::setType (InfoType info) {
	auto type = new (Z0)  IType (this-> token, info-> cloneOnExit ());
	return new (Z0)  ITypedVar (this-> token, type, this-> deco);	
    }
    
    Type IVar::asType () {
	std::vector <Expression> tmps;
	for (auto it : this-> templates) {
	    it-> inside = this;
	    auto tmpExpr = it-> expression ();
	    if (tmpExpr == NULL) return NULL;	    
	    tmps.push_back (tmpExpr);
	}

	if (!IInfoType::exists (this-> token.getStr ())) {
	    auto sym = Table::instance ().get (this-> token.getStr ());
	    if (sym != NULL && sym-> type ()-> isType ()) {		
		auto t_info = sym-> type ()-> TempOp (tmps);
		if (t_info && t_info-> is<IStructCstInfo> ())
		    t_info = t_info-> TempOp ({});
		if (t_info != NULL) {
		    if (this-> deco == Keys::REF)
			t_info = new (Z0)  IRefInfo (false, t_info);
		    else if (this-> deco == Keys::CONST) t_info-> isConst (true);
		    return new (Z0)  IType (this-> token, t_info);
		}
	    }
	    
	    if (!sym)
		Ymir::Error::undefVar (this-> token, Table::instance ().getAlike (this-> token.getStr ()));
	    else
		Ymir::Error::useAsType (this-> token);
	    return NULL;
	    
	} else {
	    auto t_info = IInfoType::factory (this-> token, tmps);
	    if (t_info == NULL) return NULL;
	    if (this-> deco == Keys::REF)
		t_info = new (Z0)  IRefInfo (false, t_info);
	    else if (this-> deco == Keys::CONST) t_info-> isConst (true);
	    return new (Z0)  IType (this-> token, t_info);
	}
    }
    
    bool IVar::isTypeV () {
	if (IInfoType::exists (this-> token.getStr ())) return true;
	else {
	    auto info = Table::instance ().get (this-> token.getStr ());
	    if (info) {
		return false;
	    }
	}
	return false;	
    }
    
    Type IType::asType () {
	if (this-> deco == Keys::REF && !this-> _type-> is <IRefInfo> ()) {
	    return new (Z0) IType (this-> token,
				   new (Z0) IRefInfo (false, this-> _type));
	}
	return this;
    }

    Expression IType::expression () {
	return this;
    }
    
    Expression IArrayVar::expression () {
	auto content = this-> content-> toType ();
	if (content == NULL) return NULL;
	Word tok (this-> token.getLocus (), "");
	InfoType type = new (Z0)  IArrayInfo (false, content-> info-> type ()-> clone ());;
	if (this-> len) {
	    auto size = this-> len-> expression ();
	    if (size == NULL) return NULL;
	    auto ul = new (Z0)  ISymbol (this-> token, this, new (Z0)  IFixedInfo (true, FixedConst::ULONG));
	    auto cmp = size-> info-> type ()-> CompOp (ul-> type ());
	    if (cmp == NULL) {
		Ymir::Error::incompatibleTypes (this-> token, size-> info, ul-> type ());
		return NULL;
	    }
		
	    if (!size-> info-> isImmutable ()) {
		Ymir::Error::notImmutable (this-> token, size-> info);
		return NULL;
	    } else {
		type-> to<IArrayInfo> ()-> isStatic (true, size-> info-> value ()-> to<IFixedValue> ()-> getUValue ());
	    }
	}
	
	tok.setStr (this-> token.getStr () + this-> content-> token.getStr () + "]");	    
	if (this-> deco == Keys::REF) {
	    type = new (Z0) IRefInfo (false, type);
	} else if (this-> deco == Keys::CONST)
	    type-> isConst (true);
	    
	return new (Z0)  IType (tok, type);
    }

    Var IArrayVar::var () {
	return this-> expression ()-> to <IVar> ();
    }

    Type IArrayVar::asType () {
	auto ret = this-> expression ();
	if (!ret) return NULL;
	return ret-> to <IType> ();
    }

    bool IArrayVar::isTypeV () {
	return true;
    }

    std::string IArrayVar::prettyPrint () {
	if (auto v = this-> content-> to <IVar> ()) {
	    if (this-> len == NULL) 
		return Ymir::format ("[%]", v-> prettyPrint ().c_str ());
	    return Ymir::format ("[% ; %]", v-> prettyPrint ().c_str (), this-> len-> prettyPrint ().c_str ());
	} else {
	    Ymir::Error::assert ("TODO");
	    return "";
	}
    }
    
    semantic::InfoType ITypedVar::getType () {
	this-> type-> inside = this;
	auto type = this-> type-> toType ();
	if (type == NULL) return NULL;
	if (this-> deco == Keys::REF && !type-> info-> type ()-> is <IRefInfo> ()) {
	    return new (Z0)  IRefInfo (false, type-> info-> type ());
	} else if (this-> deco == Keys::CONST) {
	    type-> info-> type ()-> isConst (true);
	}
	return type-> info-> type ();	    
    }

    Expression ITypedVar::expression () {
	TypedVar aux = NULL;
	auto info = Table::instance ().get (this-> token.getStr ());
	if (info && Table::instance ().sameFrame (info)) {
	    Ymir::Error::shadowingVar (this-> token, info-> sym);
	    return NULL;
	}
	
	auto type = this-> type-> toType ();
	if (type == NULL) return NULL;
	if (type-> info-> type ()-> is <IVoidInfo> ()) {
	    Ymir::Error::cannotBeVoid (this-> token);
	}
	aux = new (Z0)  ITypedVar (this-> token, type);
	
	if (this-> deco == Keys::REF && !aux-> type-> info-> type ()-> is <IRefInfo> ()) {
	    aux-> info = new (Z0)  ISymbol (this-> token, aux, new (Z0)  IRefInfo (false, aux-> type-> info-> type ()));
	} else {
	    aux -> info = new (Z0)  ISymbol (this-> token, aux, aux-> type-> info-> type ());
	    if (this-> deco == Keys::CONST) 
		aux-> info-> type ()-> isConst (true);
	}
	
	aux-> info-> type ()-> isType (false);
	Table::instance ().insert (aux-> info);
	return aux;    
    }
    
    Var ITypedVar::var () {
	return (Var) this-> expression ();
    }
    
    Expression IArrayAlloc::expression () {
	auto aux = new (Z0)  IArrayAlloc (this-> token, this-> type-> toType (), this-> size-> expression ());
	
	if (aux-> type == NULL) return NULL;
	if (aux-> size == NULL) return NULL;
		
	auto ul = new (Z0)  ISymbol (this-> token, this, new (Z0)  IFixedInfo (true, FixedConst::ULONG));
	auto cmp = aux-> size-> info-> type ()-> CompOp (ul-> type ());
	if (cmp == NULL) {
	    Ymir::Error::incompatibleTypes (aux-> size-> token, aux-> size-> info, ul-> type ());
	    return NULL;
	}

	auto arrayType = new (Z0)  IArrayInfo (false, aux-> type-> info-> type ()-> clone ());
	aux-> cster = cmp;
	aux-> info = new (Z0)  ISymbol (this-> token, aux, arrayType);

	if (this-> isImmutable) {
	    if (!aux-> size-> info-> isImmutable ()) {
		Ymir::Error::notImmutable (this-> token, aux-> size-> info);
		return NULL;
	    } else {
		arrayType-> isStatic (true, aux-> size-> info-> value ()-> to <IFixedValue> ()-> getUValue ()); 
	    }	
	} else if (Table::instance ().hasCurrentContext (Keys::SAFE)) {
	    Ymir::Error::allocationInSafe (this-> token);
	}
	
	return aux;
    }

    Expression IArrayAlloc::staticArray () {
	auto aux = new (Z0)  IArrayAlloc (this-> token, this-> type-> toType (), this-> size-> expression ());	
	auto ul = new (Z0)  ISymbol (this-> token, this, new (Z0)  IFixedInfo (true, FixedConst::ULONG));
	auto cmp = aux-> size-> info-> type ()-> CompOp (ul-> type ());
	if (cmp == NULL) {
	    Ymir::Error::incompatibleTypes (this-> token, aux-> size-> info, ul-> type ());
	    return NULL;
	}

	auto arrayType = new (Z0)  IArrayInfo (false, aux-> type-> info-> type ()-> clone ());
	aux-> cster = cmp;
	aux-> info = new (Z0)  ISymbol (this-> token, aux, arrayType);
	
	if (!aux-> size-> info-> isImmutable ()) {
	    Ymir::Error::notImmutable (this-> token, aux-> size-> info);
	    return NULL;
	} else {
	    arrayType-> isStatic (true, aux-> size-> info-> value ()-> to <IFixedValue> ()-> getUValue ()); 
	}	
	
	return aux;
    }
    
    Expression IUnary::expression () {
	auto elem = this-> elem-> expression ();
	if (elem == NULL) return NULL;

	if ((this-> token == Token::DMINUS || this-> token == Token::DPLUS || this-> token == Token::AND) && !elem-> isLvalue ()) {
	    Ymir::Error::notLValue (elem-> token);
	    return NULL;
	}

	auto type = elem-> info-> type ()-> UnaryOp (this-> token);
	if (type == NULL) {
	    Ymir::Error::undefinedOp (this-> token, elem-> info);
	    return NULL;
	}
	
	Unary unary = new (Z0)  IUnary (this-> token, elem);
	unary-> info = new (Z0)  ISymbol (this-> token, unary, type);
	return unary;
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
	} else if (this-> token.getStr () == Token::PIPE)
	    return bitwiseOr ();
	return normal ();
    }

    bool IBinary::simpleVerif (Binary aux) {
	if (aux-> left == NULL || aux-> right == NULL) return true;
	else if (!aux-> left-> isExpression ()) {
	    Ymir::Error::useAsVar (aux-> left-> token, aux-> left-> info);
	    return true;
	} else if (!aux-> right-> isExpression ()) {
	    Ymir::Error::useAsVar (aux-> right-> token, aux-> right-> info);
	    return true;
	} else if (aux-> right-> info == NULL) {
	    Ymir::Error::undefinedOp (this-> token, aux-> left-> info, new (Z0)  IVoidInfo ());
	    return true;
	} else if (aux-> left-> info == NULL) {
	    Ymir::Error::undefVar (aux-> left-> token, Table::instance ().getAlike (aux-> left-> token.getStr ()));
	    return true;
	} else if (aux-> right-> info-> type ()-> is<IUndefInfo> ()) {
	    Ymir::Error::uninitVar (aux-> right-> token, aux-> right-> info-> sym);
	    return true;
	} else if (aux-> left-> info-> type () == NULL || aux-> right-> info-> type () == NULL) return true;
	return false;
    }
    
    bool IBinary::canOverOpAssign (Binary aux) {
	if (aux-> left-> info-> type ()-> is <IAggregateInfo> ()) return true;
	if (aux-> left-> info-> type ()-> is <IStructInfo> ()) return true;
	if (auto ref = aux-> left-> info-> type ()-> to <IRefInfo> ())
	    return ref-> content ()-> is <IStructInfo> () || ref-> content ()-> is <IAggregateInfo> ();
	return false;
    }

    Expression IBinary::affect () {
	auto aux = new (Z0)  IBinary (this-> token, this-> left-> expression (), this-> right-> expression ());
	if (simpleVerif (aux)) return NULL;

	if (aux-> left-> isLvalue () &&
	    aux-> left-> info-> value () && aux-> right-> info-> value () && aux-> left-> info-> type ()-> isSame (aux-> right-> info-> type ())) {
		
	    aux-> left-> info-> value () = aux-> right-> info-> value ();
	    return aux-> left;
	} else if ((!aux-> left-> isLvalue () || aux-> left-> info-> isConst ()) && !aux-> left-> info-> type ()-> is <IUndefInfo> ()) {
	    bool fail = true;
	    if (auto ref = aux-> left-> info-> type ()-> to<IRefInfo> ())  {
		if (ref-> content ()-> is <IUndefInfo> ()) fail = false;
	    } else if (aux-> left-> info-> type () -> is <IMethodInfo> ()) fail = false;
	    
	    if (fail) {
		Ymir::Error::notLValue (aux-> left-> token);
		return NULL;
	    }
	}

	if (canOverOpAssign (aux)) {
	    if (auto call = findOpAssign (aux, false))
		return call;
	}
	
	auto type = aux-> left-> info-> type ()-> BinaryOp (this-> token, aux-> right);	
	if (type == NULL) {
	    if (auto v = aux-> left-> to <IVar> ()) {
		if (v-> fromClosure () && v-> lastInfo ()-> type ()-> is <IUndefInfo> ()) {
		    auto auxVar = new (Z0) IVar (v-> token);
		    auxVar-> info = v-> lastInfo (); 
		    type = aux-> right-> info-> type ()-> BinaryOpRight (this-> token, auxVar);
		    v-> lastInfo ()-> type (type);
		    v-> lastInfo ()-> isConst (false);
		    if (Table::instance ().retInfo ().closureMoved ()) 
			aux-> left-> info-> type (type-> clone ());
		    else {
			aux-> left-> info-> type (new (Z0) IRefInfo (false, type-> clone ()));

		    }
		    return this-> affect ();
		}
	    }  
	    type = aux-> right-> info-> type ()-> BinaryOpRight (this-> token, aux-> left);	   
	    if (aux-> left-> info-> type ()-> is<IUndefInfo> ()) {
		if (type == NULL) {
		    Ymir::Error::undefinedOp (this-> token, aux-> left-> info, aux-> right-> info);
		    return NULL;
		}
		
		aux-> left-> info-> type (type);
		aux-> left-> info-> isConst (aux-> right-> info-> type ()-> needKeepConst ());
	    } else if (type == NULL) {
		Ymir::Error::undefinedOp (this-> token, aux-> left-> info, aux-> right-> info);
		return NULL;			    
	    }
	    aux-> isRight = true;
	}
	
	aux-> info = new (Z0)  ISymbol (aux-> token, aux, type);
	Table::instance ().retInfo ().changed () = true;
	aux-> info-> value () = NULL;
	return aux;	
    }

    Expression IBinary::reaff () {
	auto aux = new (Z0)  IBinary (this-> token, this-> left-> expression (), this-> right-> expression ());
	
	if (simpleVerif (aux)) return NULL;
	if (aux-> left-> info-> type ()-> is<IUndefInfo> ()) {
	    Ymir::Error::uninitVar (aux-> left-> token, aux-> left-> info-> sym);
	    return NULL;
	} else if (!aux-> left-> isLvalue () || aux-> left-> info-> isConst ()) {
	    Ymir::Error::notLValue (aux-> left-> token);
	    return NULL;
	}
	
	if (canOverOpAssign (aux)) {
	    if (auto call = findOpAssign (aux, false))
		return call;
	}
	
	auto type = aux-> left-> info-> type ()-> BinaryOp (this-> token, aux-> right);
	if (type == NULL) {
	    type = aux-> right-> info-> type ()-> BinaryOpRight (this-> token, aux-> left);
	    if (type == NULL) {

		Ymir::Error::undefinedOp (this-> token, aux-> left-> info, aux-> right-> info);
		return NULL;
	    }
	    aux-> isRight = true;
	}
	
	aux-> info = new (Z0)  ISymbol (aux-> token, aux, type);
	Table::instance ().retInfo ().changed () = true;
	aux-> info-> value () = NULL;
	return aux;
    }    

    Expression IBinary::bitwiseOr () {
	if (!this-> info) {
	    auto left = this-> left-> expression ();
	    if (left == NULL) return NULL;
	    if (left-> isType ()) {
		auto ltype = left-> toType (), rtype = right-> toType ();
		if (ltype == NULL || rtype == NULL) return NULL;
		TupleInfo tu  = ltype-> info-> type ()-> to <ITupleInfo> ();
		if (tu != NULL) {
		    tu-> addParam (rtype-> info-> type ()-> clone ());
		} else {
		    tu = new (Z0) ITupleInfo (false, false, true);
		    tu-> addParam (ltype-> info-> type ()-> clone ());
		    tu-> addParam (rtype-> info-> type ()-> clone ());
		}
		return new (Z0) IType (this-> token, tu);
	    } else return normal ();
	} else {
	    return normal ();
	}
    }
    
    Expression IBinary::normal (Binary aux) {	
	if (!this-> info) {
	    if (aux == NULL) {
		aux = new (Z0) IBinary (this-> token, this-> left-> expression (), this-> right-> expression ());
	    }
	    
	    if (simpleVerif (aux)) return NULL;
	    if (aux-> left-> info-> type ()-> is<IUndefInfo> ()) {
		Ymir::Error::uninitVar (aux-> left-> token, aux-> left-> info-> sym);
		return NULL;
	    }

	    auto type = aux-> left-> info-> type ()-> BinaryOp (this-> token, aux-> right);

	    if (type == NULL) {
		type = aux-> right-> info-> type ()-> BinaryOpRight (this-> token, aux-> left);
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

	    aux-> info = new (Z0)  ISymbol (aux-> token, aux, type);
	    return aux;
	} else {
	    auto aux = new (Z0)  IBinary (this-> token, this-> left, this-> right);
	    aux-> info = this-> info;
	    return aux;
	}	
    }

    Expression IBinary::findOpAssign (Binary, bool mandatory) {
	Word word (this-> token.getLocus (), Keys::OPASSIGN);
	auto var = new (Z0)  IVar (word, {new (Z0)  IString (this-> token, this-> token.getStr ())});
	auto params = new (Z0)  IParamList (this-> token, {this-> left, this-> right});
	Word tok {this-> token, Token::LPAR}, tok2 {this-> token, Token::RPAR};
	
	auto call = new (Z0)  IPar (tok, tok2, var, params, false);

	if (!mandatory) Ymir::Error::activeError (false);
	auto res = call-> expression ();
	if (!mandatory) Ymir::Error::activeError (true);
	
	return res;
    }

    Expression IBinary::findOpBinary (Binary aux) {
	if (isTest (this-> token)) return findOpTest (aux);
	else if (isEq (this-> token)) return findOpEqual (aux);
	Ymir::Error::activeError (false);
	
	Word word {this-> token, Keys::OPBINARY};
	auto var = new (Z0)  IVar (word, {new (Z0)  IString (this-> token, this-> token.getStr ())});
	auto params = new (Z0)  IParamList (this-> token, {this-> left, this-> right});
	Word tok {this-> token, Token::LPAR}, tok2 {this-> token, Token::RPAR};
	auto call = new (Z0)  IPar (tok, tok2, var, params, false);
	auto res = call-> expression ();	
	auto errors = Ymir::Error::caught ();
	Ymir::Error::activeError (true);
	
	if (errors.size () != 0) {
	    Ymir::Error::activeError (false);
	    word = Word (this-> token.getLocus (), Keys::OPBINARYR);
	    var = new (Z0)  IVar (word, {new (Z0)  IString (this-> token, this-> token.getStr ())});
	    params = new (Z0)  IParamList (this-> token, {aux-> right, aux-> left});
	    call = new (Z0)  IPar (tok, tok2, var, params, false);
	    res = call-> expression ();
	    auto errors2 = Ymir::Error::caught ();
	    Ymir::Error::activeError (true);
	    if (errors2.size () != 0) {
		for (auto it : errors) printf ("%s\n", it.msg.c_str ());
		return NULL;
	    }
	}	
	
	return res;
    }

    Expression IBinary::findOpTest (Binary) {
	Word word {this-> token.getLocus (), Keys::OPTEST};
	auto var = new (Z0) IVar (word, {new (Z0) IString (this-> token, this-> token.getStr ())});
	auto params = new (Z0) IParamList (this-> token, {this-> left, this-> right});
	Word tok {this-> token, Token::LPAR}, tok2 {this-> token, Token::RPAR};
	auto call = new (Z0) IPar (tok, tok2, var, params, false);
	auto res = call-> expression ();
	if (res == NULL) return NULL;

	if (res-> info-> type ()-> is <IBoolInfo> ()) return res;
	else if (auto dec = res-> info-> type ()-> to <IFixedInfo> ()) {
	    auto fx = new (Z0) IFixed (this-> token, dec-> type ());
	    fx-> setValue (0);
	    fx-> setUValue (0);
	    auto bin = new (Z0) IBinary (this-> token, res, fx-> expression ());
	    bin-> info = new (Z0) ISymbol (this-> token, bin, bin-> left-> info-> type ()-> BinaryOp (this-> token, bin-> right));
	    return bin;
	}
	return NULL;	
    }
    
    Expression IBinary::findOpEqual (Binary) {
	Ymir::Error::activeError (false);

	Word word (this-> token.getLocus (), Keys::OPEQUAL);
	auto var = new (Z0)  IVar (word);
	auto params = new (Z0)  IParamList (this-> token, {this-> left, this-> right});
	Word tok {this-> token, Token::LPAR}, tok2 {this-> token, Token::RPAR};
	auto call = new (Z0)  IPar (tok, tok2, var, params, false);
	Expression res = NULL;
	if (this-> token == Token::DEQUAL) {
	    res = call-> expression ();
	} else {
	    res = new (Z0) IUnary ({this-> token.getLocus (), Token::NOT}, call);
	    res = res-> expression ();
	}
	auto errors = Ymir::Error::caught ();
	Ymir::Error::activeError (true);
	
	if (errors.size () != 0) {
	    Ymir::Error::activeError (false);
	    word = Word (this-> token.getLocus (), Keys::OPEQUAL);
	    var = new (Z0)  IVar (word);
	    params = new (Z0)  IParamList (this-> token, {this-> right, this-> left});
	    call = new (Z0)  IPar (this-> token, this-> token, var, params, false);
	    if (this-> token == Token::DEQUAL) {
		res = call-> expression ();
	    } else {
		res = new (Z0) IUnary ({this-> token.getLocus (), Token::NOT}, call);
		res = res-> expression ();
	    }
	    auto errors2 = Ymir::Error::caught ();
	    Ymir::Error::activeError (true);
	    if (errors2.size () != 0) {
		for (auto it : errors) printf ("%s\n", it.msg.c_str ());
		return NULL;
	    }
	}	
	
	return res;
       
    }
    
    bool IBinary::isTest (Word elem) {
	return canFind (std::vector <std::string> {Token::INF, Token::SUP, Token::INF_EQUAL, Token::SUP_EQUAL, Token::NOT_INF, Token::NOT_SUP, Token::NOT_INF_EQUAL, Token::NOT_SUP_EQUAL}, elem.getStr ());
    }

    bool IBinary::isEq (Word elem) {
	return elem == Token::DEQUAL || elem == Token::NOT_EQUAL;
    }

    Expression IFixed::expression () {
	auto aux = new (Z0)  IFixed (this-> token, this-> type);
	aux-> info = new (Z0)  ISymbol (this-> token, aux, new (Z0)  IFixedInfo (false, this-> type));
	aux-> info-> value () = new (Z0)  IFixedValue (this-> type, this-> uvalue, this->value);
	aux-> uvalue = this-> uvalue;
	aux-> value = this-> value;
	return aux;
    }
    
    Expression IChar::expression () {
	auto aux = new (Z0)  IChar (this-> token, this-> code);
	aux-> info = new (Z0)  ISymbol (this-> token, aux, new (Z0)  ICharInfo (false));
	aux-> info-> value () = new (Z0) IFixedValue (FixedConst::UBYTE, this-> code, this-> code);
	return aux;
    }

    Expression IFloat::expression () {
	auto aux = new (Z0)  IFloat (this-> token, this-> suite, this-> _type);
	aux-> info = new (Z0)  ISymbol (this-> token, aux, new (Z0)  IFloatInfo (false, this-> _type));
	aux-> totale = this-> totale;
	char * temp;
	if (this-> _type == FloatConst::FLOAT) {
	    float val = strtof (this-> totale.c_str (), &temp); 
	    aux-> setValue (val);
	    aux-> info-> value () = new (Z0) IFloatValue (FloatConst::FLOAT, val, val);
	} else {
	    double val = strtod (this-> totale.c_str (), &temp);
	    aux-> setValue (val);	
	    aux-> info-> value () = new (Z0) IFloatValue (FloatConst::DOUBLE, val, val);
	}
	return aux;
    }

    Expression IString::expression () {
	auto aux = new (Z0)  IString (this-> token, this-> content);
	auto arrayType = new (Z0) IStringInfo (false, true);
	aux-> info = new (Z0)  ISymbol (this-> token, aux, arrayType);
	aux-> info-> value () = new (Z0)  IStringValue (this-> content);	
	return aux;
    }
    
    Expression ICast::expression () {
	Expression type;
	if (auto v = this-> type-> to<IVar> ()) {
	    type = v-> asType ();
	} else type = this-> type-> expression ();

	auto expr = this-> expr-> expression ();
	if (!type || !expr) return NULL;
	else if (!expr-> isExpression ()) {
	    Ymir::Error::useAsVar (expr-> token, expr-> info);
	    return NULL;
	} else if (!type-> isType ()) {
	    Ymir::Error::useAsType (type-> token);
	    return NULL;
	}

	if (expr-> info-> type ()-> isSame (type-> info-> type ())) {
	    return expr;
	} else {
	    auto info = expr-> info-> type ()-> CastOp (type-> info-> type ());
	    if (info == NULL) {
		info = expr-> info-> type ()-> CompOp (type-> info-> type ());
		if (info == NULL) {
		    Ymir::Error::undefinedOp (this-> token, expr-> info, type-> info-> type ());
		    return NULL;
		}
	    } else if (info == expr-> info-> type ())
		return expr;
	    
	    if (expr-> info-> isConst ())
		info-> isConst (true);
	    else info-> isConst (type-> info-> isConst ());

	    auto aux = new (Z0)  ICast (this-> token, type, expr);
	    aux-> info = new (Z0)  ISymbol (this-> token, aux, info);
	    return aux;
	}	    
    }

    Expression IConstArray::expression () {
	auto aux = new (Z0)  IConstArray (this-> token, {});
	if (this-> params.size () == 0) {
	    aux-> info = new (Z0)  ISymbol (aux-> token, aux, new (Z0)  IArrayInfo (false, new (Z0)  IVoidInfo ()));	    
	} else {
	    auto val = new (Z0) IArrayValue ();
	    for (uint i = 0 ; i < this-> params.size (); i++) {
		auto expr = this-> params [i]-> expression ();
		if (expr == NULL) return NULL;

		if (auto par = expr-> to<IParamList> ()) {
		    for (auto it : par-> getParams ())
			aux-> params.push_back (it);
		} else aux-> params.push_back (expr);

		if (expr-> info-> value ()) {
		    val-> addValue (expr-> info-> value ());
		} else {
		    delete val;
		    val = NULL;
		}
	    }

	    if (aux-> params.size () == 1)  {
		auto expr = aux-> params [0]-> expression ();
		if (auto type = expr-> to <IType> ()) {
		    Word tok (this-> token.getLocus (),
			      this-> token.getStr () + type-> token.getStr () + "]"
		    );
		    return new (Z0)  IType (tok, new (Z0)  IArrayInfo (false, type-> info-> type ()));
		} else if (expr-> info-> type ()-> isType ()) {
		    Word tok (this-> token.getLocus (),
			      this-> token.getStr () + expr-> token.getStr () + "]"
		    );
		    auto inner = expr-> info-> type ();
		    if (expr-> info-> type ()-> is <IEnumCstInfo> ()) inner = inner-> TempOp ({});
		    else if (expr-> info-> type ()-> is <IStructCstInfo> ()) inner = inner-> TempOp ({});
		    return new (Z0)  IType (tok, new (Z0)  IArrayInfo (false, inner));				
		}
	    }

	    auto type = aux-> validate ();
	    if (!type) return NULL;	    
	    type-> isConst (false);
	    auto arrayType = new (Z0)  IArrayInfo (false, type);
	    
	    aux-> info = new (Z0)  ISymbol (aux-> token, aux, arrayType);
	    aux-> info-> value () = val;
	    arrayType-> isStatic (true, aux-> params.size ());	    
	}
	return aux;
    }

    InfoType IConstArray::validate () {
	if (this-> params.size () == 0) return new (Z0) IVoidInfo ();
	this-> casters.clear ();
	InfoType successType = NULL;
	for (auto fst : Ymir::r (0, this-> params.size ())) {
	    std::vector <InfoType> casters (this-> params.size ());;
	    if (!this-> params [fst]-> isExpression ()) {
		Ymir::Error::useAsVar (this-> params [fst]-> token,
				       this-> params [fst]-> info);
		return NULL;
	    }

	    auto begin = this-> params [fst]-> info-> type ();
	    casters [fst] = begin-> CompOp (new (Z0)  IUndefInfo ());	    
	    bool success = true;
	    for (auto scd : Ymir::r (0, this-> params.size ())) {
		if (scd != fst) {
		    casters [scd] = this-> params [scd]-> info-> type ()-> CompOp (begin);
		    if (casters [scd])
			casters [scd] = casters [scd]-> ConstVerif (begin);
		}
		
		if (casters [scd] == NULL) {
		    success = false;
		    break;
		}
	    }

	    if (success) {
		this-> casters = casters;
		successType = casters [fst];
		break;
	    }	    
	}

	if (this-> casters.size () != this-> params.size ()) {
	    for (auto it : Ymir::r (1, this-> params.size ())) {
		if (this-> params [it]-> info-> type ()-> CompOp (this-> params [0]-> info-> type ()) == NULL) {
		    Ymir::Error::incompatibleTypes (this-> token, this-> params [0]-> info, this-> params [it]-> info-> type ());
		    return NULL;
		}
	    }
	    return NULL;
	} else {	    
	    return successType-> clone ();
	}
    }

    
    Expression IConstRange::expression () {
	auto aux = new (Z0)  IConstRange (this-> token, this-> left-> expression (), this-> right-> expression ());
	if (!aux-> left || !aux-> right) return NULL;
	auto type = aux-> left-> info-> type ()-> BinaryOp (this-> token, aux-> right);
	if (type == NULL) {
	    auto call = findOpRange (aux);
	    if (!call) {
		Ymir::Error::undefinedOp (this-> token, aux-> left-> info, aux-> right-> info);
		return NULL;
	    } else {
		return call;
	    }
	}

	aux-> info = new (Z0)  ISymbol (aux-> token, aux, new (Z0)  IRangeInfo (true, type));
	return aux;
    }

    Expression IConstRange::findOpRange (ConstRange aux) {
	Ymir::Error::activeError (false);
	Word word (this-> token.getLocus (), Keys::OPRANGE);
	auto var = new (Z0)  IVar (word);
	auto params = new (Z0)  IParamList (this-> token, {aux-> left, aux-> right});
	
	auto call = new (Z0)  IPar (this-> token, this-> token, var, params, false);

	auto res = call-> expression ();
	auto errors = Ymir::Error::caught ();
	Ymir::Error::activeError (true);
	
	if (errors.size () != 0) return NULL;
	else return res;	
    }
    
    Expression IDColon::expression () {
	this-> left-> inside = this;
	auto left = this-> left-> expression ();
	if (left == NULL) return NULL;
	if (left-> info-> type ()-> is<IUndefInfo> ()) {
	    Ymir::Error::uninitVar (left-> token, left-> info-> sym);
	    return NULL;
	}
	
	if (auto mod = left-> info-> type ()-> to<IModuleInfo> ()) {
	    auto content = mod-> get ();
	    if (content == NULL) {
		Ymir::Error::undefAttr (this-> token, left-> info, this-> right-> to<IVar> ());
		return NULL;
	    }
	    
	    bool needToClose = false;
	    auto space = Table::instance ().space ();
	    if (!content-> authorized (space)) {
		content-> addOpen (space);
		needToClose = true;
	    }
	    
	    auto sym = content-> getFor (this-> right-> token.getStr (), space);
	    if (sym == NULL) {
		Ymir::Error::undefAttr (this-> token, left-> info, this-> right-> to<IVar> ());
		return NULL;
	    }
	    
	    this-> right-> inside = this;
	    auto aux = this-> right-> to<IVar> ()-> expression (sym);
	    if (needToClose) content-> close (space);

	    if (aux) {
		if (aux-> info-> treeDecl ().isNull ())
		    aux-> info = new (Z0) ISymbol (aux-> token, aux, aux-> info-> type ()-> onlyInMod (content));
	    }

	    return aux;
	} else {
	    if (!this-> right-> is<IVar> ()) {
		Ymir::Error::useAsVar (this-> right-> token, this-> right-> expression ()-> info);
		return NULL;
	    }
	    
	    auto var = this-> right-> to<IVar> ();
	    auto type = left-> info-> type ()-> DColonOp (var);
	    if (type == NULL) {
		Ymir::Error::undefAttr (this-> token, left-> info, var);
		return NULL;
	    }

	    if (type-> isType ()) return new (Z0) IType (this-> token, type);
	    else {
		auto aux = new (Z0)  IDColon (this-> token, this-> left-> expression (), this-> right);
		aux-> info = new (Z0)  ISymbol (aux-> token, aux, type);
		return aux;
	    }
	}
    }

    Expression IDot::expression () {
	auto aux = new (Z0)  IDot (this-> token, this-> left-> expression (), this-> right-> templateExpReplace ({}));
	if (aux-> left == NULL) return NULL;
	else if (aux-> left-> info-> type ()-> is<IUndefInfo> ()) {
	    Ymir::Error::uninitVar (aux-> left-> token, aux-> left-> info-> sym);
	    return NULL;
	} else if (auto var = aux-> right-> to<IVar> ()) {
	    auto type = aux-> left-> info-> type ()-> DotOp (var);
	    if (type == NULL && ((this-> inside && this-> inside-> is <IPar> ()) || var-> hasTemplate ())) {
		var-> inside = aux;
		auto call = var-> expression ();
		if (call == NULL || call-> isType () || call-> info-> type ()-> is<IUndefInfo> ()) {
		    Ymir::Error::undefAttr (this-> token, aux-> left-> info, var);
		    return NULL;
		}
		return (new (Z0)  IDotCall (this-> inside, this-> right-> token, call, aux-> left))-> expression ();
	    } else if (type == NULL) {
		Ymir::Error::undefAttr (this-> token, aux-> left-> info, var);
		return NULL;		
	    } else if (type-> is <IMethodInfo> () && type-> to <IMethodInfo> ()-> isAttribute () && ((this-> inside && !this-> inside-> is <IPar> ()))) {
		auto isLeftAff = this-> inside-> is <IBinary> () && this-> inside-> to <IBinary> ()-> getLeft () == this && this-> inside-> token.getStr () == Token::EQUAL;
		if (!isLeftAff) {
		    type-> to <IMethodInfo> ()-> eraseNonAttrib ();
		    aux-> info = new (Z0)  ISymbol (aux-> token, aux, type);
		    auto params = new (Z0) IParamList (this-> token, {});
		    auto call = new (Z0) IPar (this-> token, this-> token, aux, params, true);
		    aux-> inside = call;
		    return call-> expression ();
		}
	    } else if (type-> is <IAliasCstInfo> ()) {
		if (aux-> left-> info-> type ()-> is <IAggregateInfo> ())
		    aux-> left-> info-> type ()-> to<IAggregateInfo> ()-> hasExemption () = true;
		else if (aux-> left-> info-> type ()-> is <IRefInfo> ())
		    aux-> left-> info-> type ()-> to <IRefInfo> ()-> content ()-> to<IAggregateInfo> ()-> hasExemption () = true;

		auto ret = type-> to <IAliasCstInfo> ()-> replace ({{Keys::SELF, new (Z0) IEvaluatedExpr (aux-> left)}})-> expression ();

		if (ret != NULL) {
		    ret-> info-> type ()-> isConst (type-> isConst ());
		    ret-> token = aux-> right-> token;
		}
		
		if (aux-> left-> info-> type ()-> is <IAggregateInfo> ())
		    aux-> left-> info-> type ()-> to<IAggregateInfo> ()-> hasExemption () = false;
		else if (aux-> left-> info-> type ()-> is <IRefInfo> ())
		    aux-> left-> info-> type ()-> to <IRefInfo> ()-> content ()-> to<IAggregateInfo> ()-> hasExemption () = false;
		return ret;
	    }
	    aux-> info = new (Z0)  ISymbol (aux-> token, aux, type);
	    return aux;
	} else {
	    aux-> right = aux-> right-> expression ();
	    if (aux-> right == NULL) return NULL;
	    auto type = aux-> left-> info-> type ()-> DotExpOp (aux-> right);
	    if (type == NULL) {
		Ymir::Error::undefinedOp (this-> token, aux-> left-> info, aux-> right-> info);
		return NULL;
	    }
	    aux-> info = new (Z0)  ISymbol (aux-> token, aux, type);
	    return aux;
	}
    }

    Expression IDotCall::expression () {
	if (!this-> inside || !this-> inside-> is<IPar> ()) {
	    auto aux = new (Z0)  IPar (this-> token, this-> token);
	    aux-> dotCall () = this;
	    Word word (this-> token.getLocus (), Keys::DPAR);
	    aux-> paramList () = new (Z0)  IParamList (this-> token, {this-> _firstPar});
	    aux-> left () = this-> _call;
	    auto type = aux-> left ()-> info-> type ()-> CallOp (aux-> left ()-> token, aux-> paramList ());
	    if (type == NULL) {
		Ymir::Error::undefinedOp (word, aux-> left ()-> info, aux-> paramList ());
		return NULL;
	    } else if (type-> ret == NULL) return NULL;

	    aux-> score () = type;
	    aux-> info = new (Z0)  ISymbol (this-> token, aux, type-> ret);	    
	    if (type-> ret-> is <IUndefInfo> ()) {
		Ymir::Error::templateInferType (aux-> left ()-> token, aux-> score ()-> token);
		return NULL;
	    }
	    return aux;
	} else {
	    return this;
	}
    }
    
    bool IPar::simpleVerif (Par& aux) {
	if (aux-> _left == NULL ||
	    aux-> params == NULL ||
	    aux-> _left-> info == NULL ||
	    aux-> _left-> info-> type () == NULL) {
	    return true;
	}
	
	aux-> _left-> inside = this;
	
	if (!aux-> _left-> isExpression ()) {
	    Ymir::Error::useAsVar (aux-> _left-> token, aux-> _left-> info);
	    return true;
	} else if (aux-> _left-> info-> type ()-> is<IUndefInfo> ()) {
	    Ymir::Error::uninitVar (aux-> _left-> token, aux-> _left-> info-> sym);
	    return true;
	}
	
	return false;
    }

    Expression IPar::expression () {
	auto aux = new (Z0)  IPar (this-> token, this-> end);
	if (this-> info == NULL) {
	    if (auto p = this-> params-> expression ()) 
		aux-> params = p-> to<IParamList> ();
	    else return NULL;
	    
	    aux-> _left = this-> _left-> expression ();	    
	    if (simpleVerif (aux)) return NULL;

	    if (auto dcall = aux-> _left-> to<IDotCall> ()) {
		aux-> _left = dcall-> call ();
		aux-> params-> getParams ().insert (aux-> params-> getParams ().begin (), dcall-> firstPar ());
		aux-> _dotCall = dcall;
	    }

	    auto type = aux-> _left-> info-> type ()-> CallOp (aux-> _left-> token, aux-> params);	    

	    if (type == NULL) {
		auto call = !this-> _opCall ? findOpCall () : NULL;
		if (call == NULL) {
		    if (this-> token.getStr () != this-> end.getStr ())
			Ymir::Error::undefinedOp (this-> token, this-> end, aux-> _left-> info, aux-> params);
		    else
			Ymir::Error::undefinedOp (this-> token, aux-> _left-> info, aux-> params);
		    return NULL;
		} else return call;
	    } else if (type-> ret == NULL) {
		return NULL;
	    }

	    if (type-> treat.size () != aux-> params-> getParams ().size ()) {
		bool need = true;
		if (aux-> _left-> info-> type ()-> is <IFunctionInfo> () && aux-> _left-> info-> type ()-> to <IFunctionInfo> ()-> isConstr ())
		    need = false;
		else if (type-> isMethod && type-> treat.size () == aux-> params-> getParams ().size () + 1)
		    need = false;

		if (need)
		    tuplingParams (type, aux);
	    }

	    aux-> _score = type;
	    aux-> info = new (Z0) ISymbol (this-> token, aux, type-> ret);

	    if (type-> ret-> is<IUndefInfo> () && this-> inside != NULL) {
		Ymir::Error::templateInferType (aux-> _left-> token, aux-> _score-> token);
		return NULL;
	    } else if (type-> ret-> is <IUndefInfo> ()) {
		type-> ret = new (Z0) IVoidInfo ();
	    }

	    if (!aux-> info-> isImmutable ())
		Table::instance ().retInfo ().changed () = true;
	    return aux;	    
	} else {
	    aux-> _left = this-> _left;
	    aux-> _score = this-> _score;
	    aux-> params = this-> params;
	    aux-> _dotCall = this-> _dotCall;
	    aux-> _opCall = this-> _opCall;
	    aux-> info = this-> info;
	    return aux;
	}
    }

    Expression IPar::findOpCall () {
	Word word (this-> token.getLocus (), Keys::OPCALL);
	auto var = new (Z0) IVar (word);
	std::vector <Expression> params = {this-> _left};
	params.insert (params.end (), this-> params-> getParams ().begin (),
		       this-> params-> getParams ().end ());

	Word tok {this-> token, Token::LPAR}, tok2 {this-> token, Token::RPAR};
	auto finalParams = new (Z0)  IParamList (this-> token, params);
	auto call = new (Z0)  IPar (tok, tok2, var, finalParams, true);
	
	return call-> expression ();
    }
    
    void IPar::tuplingParams (ApplicationScore score, Par par) {
	std::vector <Expression> lasts (par-> params-> getParams ().begin () + score-> treat.size () - 1, par-> params-> getParams ().end ());
	auto lastInfo = score-> treat.back ()-> to<ITupleInfo> ();
	auto ctuple = new (Z0) IConstTuple (par-> token, par-> token, lasts);
	ctuple-> info = new (Z0) ISymbol (par-> token, ctuple, lastInfo);
	for (auto it : lastInfo-> getParams ()) {
	    ctuple-> getCasters ().push_back (it);
	}
	
	score-> treat.back () = lastInfo-> asNoFake ();
	std::vector <Expression> alls (par-> params-> getParams ().begin (), par-> params-> getParams ().begin () + score-> treat.size () - 1);
	alls.push_back (ctuple);
	par-> params-> getParams () = alls;
    }
    
    Expression IParamList::expression () {
	auto aux = new (Z0)  IParamList (this-> token, {});
	for (auto it : Ymir::r (0, this-> params.size ())) {
	    this-> params [it]-> inside = this;
	    Expression ex_it = this-> params [it]-> expression ();
	    if (ex_it == NULL || ex_it-> info == NULL || ex_it-> info-> type () == NULL) return NULL;
	    if (auto ex = ex_it-> to<IParamList> ()) {
		for (auto it : ex-> params) {
		    aux-> params.push_back (it);
		    if (aux-> params.back ()-> info-> type ()-> is<IUndefInfo> ()) {
			Ymir::Error::uninitVar (aux-> params.back ()-> token, aux-> params.back ()-> info-> sym);
			return NULL;
		    }
		}
	    } else {
		aux-> params.push_back (ex_it);
		if (ex_it-> info-> type ()-> is<IUndefInfo> ()) {
		    Ymir::Error::uninitVar (ex_it-> token, ex_it-> info-> sym);
		    return NULL;
		} else if (!ex_it-> isExpression ()) {
		    Ymir::Error::useAsVar (ex_it-> token, ex_it-> info);
		    return NULL;
		} 
	    }
	}
	return aux;
    }   
    
    Expression IBool::expression () {
	auto aux = new (Z0)  IBool (this-> token);
	aux-> value = this-> value;
	aux-> info = new (Z0)  ISymbol (this-> token, aux, new (Z0)  IBoolInfo (false));
	aux-> info-> value () = new (Z0)  IBoolValue (this-> value);
	return aux;
    }

    Expression INull::expression () {
	auto aux = new (Z0)  INull (this-> token);
	aux-> info = new (Z0)  ISymbol (this-> token, aux, new (Z0)  INullInfo ());
	return aux;
    }

    Expression IIgnore::expression () {
	auto aux = new (Z0)  IIgnore (this-> token);
	aux-> info = new (Z0)  ISymbol (this-> token, aux, new (Z0)  IIgnoreInfo ());
	return aux;
    }
    
    Expression IConstTuple::expressionFake () {
	auto aux = new (Z0) IConstTuple (this-> token, this-> end, {});
	auto type = new (Z0) ITupleInfo (false, true);
	for (auto it : this-> params) {
	    auto expr = it-> expression ();
	    if (expr == NULL) return NULL;
	    if (auto par = expr-> to <IParamList> ()) {
		for (auto exp_it : par-> getParams ()) {
		    aux-> params.push_back (exp_it);
		    type-> addParam (exp_it-> info-> type ());
		}
	    } else {
		aux-> params.push_back (expr);
		type-> addParam (expr-> info-> type ());
	    }
	}
	aux-> isFake () = true;
	aux-> info = new (Z0) ISymbol (this-> token, aux, type);
	return aux;
    }
    
    Expression IConstTuple::expression () {
	if (this-> isFake ()) return expressionFake ();
	auto aux = new (Z0)  IConstTuple (this-> token, this-> end, {});
	auto type = new (Z0)  ITupleInfo (false);
	//auto undef = new (Z0)  IUndefInfo ();
	for (auto it : this-> params) {
	    auto expr = it-> expression ();
	    if (expr-> isType ()) return this-> asType ();
	    if (expr == NULL) return NULL;
	    if (auto par = expr-> to <IParamList> ()) {
		for (auto exp_it : par-> getParams ()) {
		    aux-> casters.push_back (exp_it-> info-> type ()-> CompOp (exp_it-> info-> type ()));
		    if (aux-> casters.back () == NULL) {
			Ymir::Error::incompatibleTypes (exp_it-> token, exp_it-> info, exp_it-> info-> type ());
			return NULL;
		    }
		    aux-> params.push_back (exp_it);
		    type-> addParam (exp_it-> info-> type ());
		}
	    } else {		
		aux-> casters.push_back (expr-> info-> type ()-> CompOp (expr-> info-> type ()));
		if (aux-> casters.back () == NULL) {
		    Ymir::Error::incompatibleTypes (expr-> token, expr-> info, expr-> info-> type ());
		    return NULL;
		}
		aux-> params.push_back (expr);
		type-> addParam (expr-> info-> type ());
	    }
	}
	aux-> info = new (Z0)  ISymbol (this-> token, aux, type);
	return aux;	
    }

    Expression IConstTuple::asType () {
	auto type = new (Z0) ITupleInfo (false);
	for (auto it : this-> params) {
	    auto expr = it-> toType ();
	    if (expr == NULL) return NULL;
	    type-> addParam (expr-> info-> type ());
	}
	return new (Z0) IType (this-> token, type);
    }

    bool IConstTuple::isType () {
	for (auto it : this-> params)
	    if (!it-> isType ()) return false;
	return true;
    }
    
    Expression IExpand::expression () {
	if (this-> info) return this;
	auto expr = this-> expr-> expression ();
	if (expr == NULL) return NULL;
	if (!expr-> isExpression ()) {
	    Ymir::Error::useAsVar (expr-> token, expr-> info);
	    return NULL;
	}

	if (auto tu = expr-> to <IConstTuple> ()) {
	    if (tu-> isFake ()) {
		std::vector <Expression> params;
		for (auto it : Ymir::r (0, tu-> getExprs ().size ())) {
		    params.push_back (tu-> getExprs () [it]);
		}
		
		auto aux = new (Z0) IParamList (this-> token, params);
		aux-> info = new (Z0) ISymbol (this-> token, aux, new (Z0) IUndefInfo ());
		return aux;
	    }
	}
	
	if (auto tuple = expr-> info-> type ()-> to <ITupleInfo> ()) {
	    std::vector <Expression> params;
	    for (auto it : Ymir::r (0, tuple-> getParams ().size ())) {
		auto exp = new (Z0)  IExpand (this-> token, expr, it);
		exp-> info = new (Z0)  ISymbol (exp-> token, exp, tuple-> getParams () [it]-> clone ());
		params.push_back (exp);
	    }

	    auto aux = new (Z0)  IParamList (this-> token, params);
	    aux-> info = new (Z0)  ISymbol (this-> token, aux, new (Z0)  IUndefInfo ());
	    return aux;
	} else {
	    return expr;
	}	
    }
    
    Expression ILambdaFunc::expression () {
	auto space = Table::instance ().space ();
	auto aux = new (Z0) ILambdaFunc (this-> token, {});
	if (this-> expr)
	    aux-> expr = this-> expr-> templateExpReplace ({});
	if (this-> block) aux-> block = (Block) this-> block-> templateReplace ({});
	if (this-> ret) aux-> ret = (Var) this-> ret-> templateExpReplace ({});
	bool isPure = true;
	for (auto it : this-> params) {
	    aux-> params.push_back ((Var) it-> templateExpReplace ({}));
	    if (!aux-> params.back ()-> is <ITypedVar> ())
		isPure = false;
	}
	
	if (this-> frame.size () == 0) {
	    auto ident = Ymir::OutBuffer ("Lambda_", this-> id).str ();
	    auto fr = new (Z0) ILambdaFrame (space, ident, aux);
	    fr-> isPure (isPure);
	    fr-> isMoved () = this-> _isMoved;
	    aux-> frame = {fr};
	} else {
	    aux-> frame = this-> frame;
	}
	
	auto fun = new (Z0) IFunctionInfo (aux-> frame [0]-> space (), "", aux-> frame);	
	fun-> isLambda () = true;
	fun-> value () = new (Z0) ILambdaValue (aux-> frame);
	aux-> info = new (Z0) ISymbol (aux-> token, aux, fun);
	return aux;
    }
    
    Expression IFuncPtr::expression () {
	std::vector <Expression> tmps (this-> params.size () + 1);
	if (this-> ret) tmps [0] = this-> ret-> toType ();
	else Ymir::Error::assert ("ERROR");
	if (tmps [0] == NULL) return NULL;
	
	auto ret = tmps [0];
	std::vector <Expression> params;
	for (auto it : Ymir::r (0, this-> params.size ())) {
	    tmps [it + 1] = this-> params [it]-> toType ();
	    if (tmps [it + 1] == NULL) return NULL;
	    params.push_back (tmps [it + 1]);
	}

	auto t_info = IInfoType::factory (this-> token, tmps);
	if (t_info == NULL) return NULL;
	if (this-> expr) {
	    auto aux = this-> expr-> expression ();
	    if (aux == NULL) return NULL;
	    auto treat = aux-> info-> type ()-> CompOp (t_info);
	    if (treat == NULL) {
		Ymir::Error::incompatibleTypes (this-> token, new (Z0) ISymbol (this-> token, NULL, t_info), aux-> info-> type ());
		return NULL;
	    }

	    auto func = new (Z0) IFuncPtr (this-> token, params, ret, aux);
	    func-> info = new (Z0) ISymbol (this-> token, func, treat);
	    return func;
	} else {
	    return new (Z0) IType (this-> token, t_info);
	}
    }

    Expression IStructCst::expression () {
	auto aux = new (Z0) IStructCst (this-> token, this-> end);
	this-> left-> inside = this;
	this-> params-> inside = this;

	aux-> params = (ParamList) this-> params-> expression ();
	aux-> left = this-> left-> expression ();
	if (aux-> left == NULL) return NULL;
	if (aux-> params == NULL) return NULL;
	if (!aux-> left-> isType ()) {
	    Ymir::Error::useAsType (this-> left-> token);
	    return NULL;
	}

	auto type = aux-> left-> info-> type ()-> clone ();
	type-> isType (true);
	auto score = type-> CallType (aux-> left-> token, aux-> params);
	if (score == NULL) {
	    Ymir::Error::undefinedOp (this-> token, this-> end, aux-> left-> info, aux-> params);
	    return NULL;
	}

	aux-> score = score;
	aux-> info = new (Z0) ISymbol (this-> token, aux, score-> ret);
	return aux;	
    }
    
    Expression IIs::expression () {
	if (this-> type) {
	    auto aux = new (Z0) IIs (this-> token, this-> left-> expression (), this-> type-> toType ());
	    if (aux-> type == NULL || aux-> left == NULL) return NULL;
	    
	    auto rtype = aux-> type-> info-> type ();
	    while (auto ref = rtype-> to <IRefInfo> ()) rtype = ref-> content ();
	    
	    if (aux-> left-> info-> type ()-> is<IUndefInfo> ()) {
		Ymir::Error::uninitVar (aux-> left-> token, aux-> left-> info-> sym);
		return NULL;
	    }
	    
	    auto linfo = aux-> left-> info-> type ();
	    if (auto ref = linfo-> to <IRefInfo> ())
		linfo = ref-> content ();

	    auto res = linfo-> isSame (rtype);
	    auto type = new (Z0) IBoolInfo (true);
	    aux-> info = new (Z0) ISymbol (this-> token, aux, type);
	    aux-> info-> value () = new (Z0) IBoolValue (res);
	    return aux;
	} else {
	    auto aux = new (Z0) IIs (this-> token, this-> left-> expression (), this-> expType);
	    if (aux-> left == NULL) return NULL;
	    else if (aux-> left-> info-> type ()-> is<IUndefInfo> ()) {
		Ymir::Error::uninitVar (aux-> left-> token, aux-> left-> info-> sym);
		return NULL;
	    }
	    auto type = new (Z0) IBoolInfo (true);
	    aux-> info = new (Z0) ISymbol (this-> token, aux, type);
	    if (this-> expType == Keys::FUNCTION) {
		aux-> info-> value () = new (Z0) IBoolValue (
		    aux-> left-> info-> type ()-> is <IPtrFuncInfo> () ||
		    aux-> left-> info-> type ()-> is <IFunctionInfo> ()
		);
	    } else if (this-> expType == Keys::TUPLE) {
		aux-> info-> value () = new (Z0) IBoolValue (
		    aux-> left-> info-> type ()-> is <ITupleInfo> ()
		);
	    } else if (this-> expType == Keys::STRUCT) {
		aux-> info-> value () = new (Z0) IBoolValue (
		    aux-> left-> info-> type ()-> is <IStructInfo> () ||
		    aux-> left-> info-> type ()-> is <IStructCstInfo> () 
		);
	    } else {
		Ymir::Error::assert ("");
		return NULL;
	    }
	    return aux;
	}
    }

    Expression ITypeOf::expression () {
	auto expr = this-> expr-> expression ();
	if (expr == NULL) return NULL;
	auto type = expr-> info-> type ()-> cloneOnExit ();
	while (auto ref = type-> to <IRefInfo> ()) type = ref-> content ();
	
	if (this-> _mut) type-> isConst (false);	
	return new (Z0) IType (this-> token, type);
    }

    Expression IStringOf::expression () {
	std::string val = this-> expr-> prettyPrint ();
	auto ret = new (Z0) IString (this-> token, val);
	ret-> info = new (Z0) ISymbol (this-> token, ret, new (Z0) IStringInfo (true, true));
	ret-> info-> value () = new (Z0) IStringValue (val);
	return ret;
    }
    
    std::vector <semantic::Symbol> IMatch::allInnerDecls () {
	std::vector <Symbol> syms;
	for (auto it : this-> soluce) {
	    for (auto it_ : it.created)
		syms.push_back (it_-> info);
	}
	return syms;
    }
    
    Expression IMatch::expression () {
	Table::instance ().enterBlock ();
	auto aux = new (Z0) IVar ({expr-> token, "_"});
	auto expr = this-> expr-> expression ();
	if (expr == NULL) return NULL;
	
	aux-> info = new (Z0) ISymbol (aux-> token, aux, new (Z0) IRefInfo (expr-> info-> isConst ()
								       , expr-> info-> type ()-> clone ()));
	Table::instance ().insert (aux-> info);
	
	Word affTok {this-> token, Token::EQUAL};
	auto binAux = (new (Z0) IAffectGeneric (affTok, aux, expr, true))-> expression ();
	if (!binAux) return NULL;       
	aux-> info-> value () = expr-> info-> value ();
	
	std::vector <semantic::DestructSolution> soluce;
	std::vector <Block> results;
	std::vector <Symbol> syms;
	//bool unreachable = false;
	for (auto it : Ymir::r (0, this-> values.size ())) {
	    auto res = semantic::DestructSolver::instance ().solve (this-> values [it], aux);
	    if (res.valid) {
		soluce.push_back (res);
		Table::instance ().enterBlock ();
		for (auto it : res.created) {
		    Table::instance ().insert (it-> info);
		}

		auto bl = this-> block [it]-> block ();
		auto expr = bl-> getLastExpr ();
		if (expr == NULL) {
		    Ymir::Error::uninitVar (bl-> token, bl-> token);
		    return NULL;
		}
		
		results.push_back (bl);
		syms.push_back (expr-> info);
		Table::instance ().quitBlock ();
	    }
	    //	    if (res.immutable) unreachable = true;	    
	}
	
	Table::instance ().quitBlock ();
	auto ret = new (Z0) IMatch (this-> token, expr);
	ret-> aux = aux;
	ret-> binAux = binAux;
	ret-> soluce = soluce;
	ret-> block = results;
	auto type = ret-> validate (syms);
	if (type == NULL) return NULL;
	ret-> info = new (Z0) ISymbol (this-> token, ret, type);
	return ret;
    }

    InfoType IMatch::validate (std::vector <Symbol> & params) {
	if (params.size () == 0) return new (Z0) IVoidInfo ();
	this-> casters.clear ();
	InfoType successType = NULL;
	for (auto fst : Ymir::r (0, params.size ())) {
	    std::vector <InfoType> casters (params.size ());
	    auto begin = params [fst]-> type (); 
	    casters [fst] = begin-> CompOp (new (Z0) IUndefInfo ());
	    bool success = true;
	    for (auto scd : Ymir::r (0, params.size ())) {
		if (scd != fst) {
		    casters [scd] = params [scd]-> type ()-> CompOp (begin);
		    if (casters [scd])
			casters [scd] = casters [scd]-> ConstVerif (begin);
		}
		if (casters [scd] == NULL) {
		    success = false;
		    break;
		}
	    }

	    if (success) {
		this-> casters = casters;
		successType = casters [fst];
		break;
	    }	    
	}

	if (this-> casters.size () != params.size ()) {
	    for (auto it : Ymir::r (1, params.size ())) {
		if (!params [it]-> type ()-> CompOp (params [0]-> type ())) {
		    Ymir::Error::incompatibleTypes (this-> token, params [it], params [0]-> type ());
		    return NULL;
		}		
	    }
	    return NULL;
	} else {
	    return successType-> clone ();
	}		
    }


    Expression IPragma::expression () {
	if (this-> token.getStr () == COMPILE) return executeCompile ();
	if (this-> token.getStr () == MSG) executeMsg ();

	auto ret = new (Z0) IPragma (this-> token, NULL);
	ret-> info = new (Z0) ISymbol (this-> token, ret, new (Z0) IVoidInfo ());
	return ret;
    }

    void IPragma::executeMsg () {
	Ymir::OutBuffer buf;
	auto l = this-> params-> expression ();
	if (l) {
	    auto list = l-> to <IParamList> ();
	    for (auto it : list-> getParams ()) {
		if (it-> info-> isImmutable ())
		    buf.write (it-> info-> value ()-> toString ());
		else Ymir::Error::notImmutable (it-> token, it-> info);
	    }
	    println (buf.str ());
	}
    }

    Expression IPragma::executeCompile () {
	Ymir::Error::activeError (false);
	this-> params-> expression ();
	auto errors = Ymir::Error::caught ();
	Ymir::Error::activeError (true);
	auto ret = new (Z0) IPragma (this-> token, NULL);
	ret-> info = new (Z0) ISymbol (this-> token, ret, new (Z0) IBoolInfo (true));
	if (errors.size () != 0) {
	    ret-> info-> value () = new (Z0) IBoolValue (false);	    
	} else
	    ret-> info-> value () = new (Z0) IBoolValue (true);
	return ret;
	
    }
    
    Expression IMacroCall::expression () {
	if (auto block = this-> bl) {
	    if (!Table::instance ().addCall (this-> token)) return NULL;
	    auto nbErrors = Ymir::Error::nb_errors;
	    Table::instance ().enterPhantomBlock ();
	    Table::instance ().retInfo ().info = new (Z0) ISymbol (this-> token, NULL, new (Z0) IVoidInfo ());
	    auto aux = block-> expression ();	    
	    Table::instance ().quitFrame ();
	    
	    if (Ymir::Error::nb_errors != nbErrors) {
		Ymir::OutBuffer buf (this-> soluce);		
		Ymir::Error::templateCreation (this-> left-> token, buf.str ());		
		return NULL;
	    }
	    
	    return aux;
	} else {
	    auto res = this-> solve ({});
	    if (!res) return NULL;
	    return res-> expression ();
	}	
	return NULL;
    }

    Expression IMacroVar::expression () {
	return this-> content-> expression ();
    }

    Expression IBlock::expression () {
	auto bl = this-> block ();
	auto expr = bl-> getLastExpr ();	
	if (expr != NULL && expr-> info && !expr-> info-> type ()-> is <IVoidInfo> ()) {
	    bl-> info = new (Z0) ISymbol (bl-> token, bl, expr-> info-> type ()-> clone ());
	    bl-> value = expr;
	    bl-> insts.pop_back ();
	} else {
	    bl-> info = new (Z0) ISymbol (bl-> token, bl, new (Z0) IVoidInfo ());
	}
	return bl;
    }
    
    Expression IMacroRepeat::expression () {
	auto res = this-> templateExpReplace ({})-> to <IMacroRepeat> ();
	res-> info = new (Z0) ISymbol (this-> token, res, new (Z0) IVoidInfo ());
	return res;
    }

    Expression IMacroEnum::expression () {
	auto res = this-> templateExpReplace ({});
	res-> info = new (Z0) ISymbol (this-> token, res, new (Z0) IVoidInfo ());
	return res;
    }

    Expression IMacroToken::expression () {
	auto aux = new (Z0) IString (this-> token, this-> value);
	aux-> info = new (Z0) ISymbol (this-> token, aux, new (Z0) IStringInfo (true, true));
	aux-> info-> value () = new (Z0) IStringValue (this-> value);
	return aux;
    }

    Expression IAffectGeneric::expression () {
	auto right = this-> right-> expression ();
	auto left = this-> left-> expression ();
	
	if (left == NULL || right == NULL) return NULL;
	auto ret = new (Z0) IAffectGeneric (this-> token, left, right, this-> _addr);
	ret-> info = new (Z0) ISymbol (this-> token, ret, new (Z0) IVoidInfo ());
	return ret;	
    }

    Expression IFakeDecl::expression () {
	auto right = this-> right-> expression ();
	auto left = this-> left-> templateExpReplace ({});
	if (right == NULL) return NULL;
	if (this-> _addr)
	    left-> info = new (Z0) ISymbol (left-> token, left, new (Z0) IRefInfo (true, right-> info-> type ()-> clone ()));
	else
	    left-> info = new (Z0) ISymbol (left-> token, left, right-> info-> type ()-> clone ());
	
	if (this-> _const)
	    left-> info-> isConst (true);
	left-> info-> isInline () = true;

	Table::instance ().insert (left-> info);
	auto ret = new (Z0) IAffectGeneric (this-> token, left, right, this-> _addr);
	ret-> info = new (Z0) ISymbol (this-> token, ret, new (Z0) IVoidInfo ());
	return ret;
    }

    Expression IEvaluatedExpr::expression () {
	return this-> _value;
    }
    
    
}
