#include <ymir/ast/_.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/syntax/Token.hh>
#include <ymir/syntax/FakeLexer.hh>
#include <ymir/syntax/Visitor.hh>
#include <ymir/semantic/value/FixedValue.hh>

using namespace std;
using namespace semantic;

namespace syntax {


    Expression paramListToTupleVar (Word token, ParamList params) {
	std::vector <Expression> rets;
	for (auto it_ : params-> getParams ()) {
	    rets.push_back (it_);
	}
	return new (Z0) IConstTuple (token, token, rets);
    }

    Expression IAccess::templateExpReplace (const map <string, Expression>& values) {
	auto params = (ParamList) this-> _params-> templateExpReplace (values);
	auto left = this-> _left-> templateExpReplace (values);
	return new (Z0)  IAccess (this-> token, this-> _end, left, params);
    }

    Expression IStructCst::templateExpReplace (const map <string, Expression>& values) {
	auto params = (ParamList) this-> params-> templateExpReplace (values);
	auto left = this-> left-> templateExpReplace (values);
	if (params == NULL || left == NULL) return NULL;
	return new (Z0) IStructCst (this-> token, this-> end, left, params);
    }    
    
    Expression IArrayAlloc::templateExpReplace (const map <string, Expression>& values) {
	auto type = this-> _type-> templateExpReplace (values);	
	auto size = this-> _size-> templateExpReplace (values);	
	return new (Z0)  IArrayAlloc (this-> token, type, size, this-> _isImmutable);
    }

    Instruction IAssert::templateReplace (const map <string, Expression>& values) {
	if (this-> _msg != NULL) {
	    return new (Z0)  IAssert (this-> token,
				     this-> _expr-> templateExpReplace (values),
				     this-> _msg-> templateExpReplace (values),
				     this-> _isStatic
	    );
	} else {
	    return new (Z0)  IAssert (this-> token,
				     this-> _expr-> templateExpReplace (values),
				     NULL,
				     this-> _isStatic
	    );
	}
    }
    
    Instruction IThrow::templateReplace (const map <string, Expression>& values) {
	return new (Z0)  IThrow (this-> token,
				 this-> _expr-> templateExpReplace (values)
	);	
    }

    
    
    Expression IBinary::templateExpReplace (const map <string, Expression>& values) {
	auto left = this-> _left-> templateExpReplace (values);
	auto right = this-> _right-> templateExpReplace (values);
	Expression autoCast = NULL;
	if (this-> _autoCaster) {
	    autoCast = this-> _autoCaster-> templateExpReplace (values);
	    if (autoCast == NULL) return NULL;
	}
	
	if (left == NULL || right == NULL) return NULL;
	return new (Z0)  IBinary (this-> token, left, right, autoCast);
    }

    Expression IBlock::templateExpReplace (const map <string, Expression>& values) {
	vector <Declaration> decls;
	vector <Instruction> insts;
	for (auto it : this-> _decls) {
	    decls.push_back (it-> templateDeclReplace (values));
	}
	
	for (auto it : this-> _insts) {
	    auto res = it-> templateReplace (values);
	    if (!res) return NULL;
	    insts.push_back (res);
	}
	
	return new (Z0)  IBlock (this-> token, decls, insts);
    }

    Instruction IScope::templateReplace (const map <string, Expression> & values) {
	return new (Z0) IScope (this-> token, (Block) this-> _block-> templateReplace (values));
    }

    Instruction IScopeFailure::templateReplace (const map <string, Expression> & values) {
	std::vector <TypedVar> vars;
	std::vector <Block> blocks;
	for (auto it : this-> _vars) vars.push_back (it-> templateExpReplace (values)-> to <ITypedVar> ());
	for (auto it : this-> _blocks) blocks.push_back (it-> templateReplace (values)-> to <IBlock> ());
	return new (Z0) IScopeFailure (this-> token, vars, blocks,
				       this-> _block != NULL ?
				       this-> _block-> templateReplace (values) -> to<IBlock> ()
				       : NULL);
    }
    
    Instruction IBreak::templateReplace (const map <string, Expression>&) {
	return new (Z0)  IBreak (this-> token, this-> _ident);	
    }

    Expression ICast::templateExpReplace (const map <string, Expression>& values) {
	auto type = this-> _type-> templateExpReplace (values);
	auto expr = this-> _expr-> templateExpReplace (values);
	return new (Z0)  ICast (this-> token, type, expr);
    }

    Expression IFixed::templateExpReplace (const map <string, Expression>&) {
	auto ret = new (Z0)  IFixed (this-> token, this-> _type, FixedMode::BUILTINS);
	ret-> setUValue (this-> _uvalue);
	ret-> setValue (this-> _value);
	return ret;
    }

    Expression IChar::templateExpReplace (const map <string, Expression>&) {
	return new (Z0) IChar (this-> token, this-> _code);
    }

    Expression IFloat::templateExpReplace (const map <string, Expression>&) {
	auto ret = new (Z0)  IFloat (this-> token, this-> _suite);
	ret-> _type = this-> _type;
	ret-> _totale = this-> _totale;
	return ret;
    }

    Expression IString::templateExpReplace (const map <string, Expression>&) {
	return new (Z0)  IString (this-> token, this-> _content);	
    }

    Expression IBool::templateExpReplace (const map <string, Expression>&) {
	return new (Z0)  IBool (this-> token);
    }

    Expression INull::templateExpReplace (const map <string, Expression>&) {
	return new (Z0)  INull (this-> token);
    }

    Expression IIgnore::templateExpReplace (const map <string, Expression>&) {
	return new (Z0)  IIgnore (this-> token);
    }

    Expression IConstArray::templateExpReplace (const map <string, Expression>& values) {
	vector <Expression> params;
	for (auto it : this-> params)
	    params.push_back (it-> templateExpReplace (values));
	return new (Z0)  IConstArray (this-> token, params);
    }
    
    Expression IDColon::templateExpReplace (const map <string, Expression>& values) {
	auto left = this-> _left-> templateExpReplace (values);
	auto right = this-> _right-> templateExpReplace (values);
	return new (Z0)  IDColon (this-> token, left, right);
    }

    Declaration IDeclaration::templateDeclReplace (const map <string, Expression>&)  {
	
	Ymir::Error::assert ("TODO");
	return NULL;
    }

    Declaration IProto::templateDeclReplace (const map <string, Expression> & exps) {
	Var type = NULL;
	if (this-> _type != NULL)
	    type = (Var) this-> _type-> templateExpReplace (exps);
	std::vector <Var> params;
	for (auto it : this-> _params)
	    params.push_back ((Var) it-> templateExpReplace (exps));
	
	auto ret = new (Z0) IProto (this-> _ident, this-> getDocs (), type, _retDeco, params, this-> _from, this-> isVariadic ());
	ret-> _from = this-> _from;
	ret-> setPublic (this-> isPublic ());
	return ret;
    }
    
    Expression IDot::templateExpReplace (const map <string, Expression>& values) {
	auto left = this-> _left-> templateExpReplace (values);
	auto right = this-> _right-> templateExpReplace (values);
	if (auto rep = left-> to<IMacroRepeat> ()) {
	    if (auto fx = right-> to <IFixed> ()) {
		auto val = fx-> expression ()-> info-> value ()-> to<semantic::IFixedValue> ()-> getValue ();
		if (val < (int) rep-> getSolution ().size ()) {
		    auto soluce = rep-> getSolution () [val];
		    for (auto & it : soluce.elements)
			it.second = it.second-> templateExpReplace (values);
		    
		    auto ret = new (Z0) IMacroRepeat (this-> token, NULL, NULL, false);
		    ret-> addSolution (soluce);
		    return ret;
		}
	    } else if (auto var = right-> to <IVar> () ) {
		if (rep-> getSolution ().size () == 1) {
		    auto name = var-> token.getStr ();
		    auto soluce = rep-> getSolution () [0];
		    auto elem = soluce.elements.find (name);
		    if (elem != soluce.elements.end ()) {
			return elem-> second-> templateExpReplace (values);
		    }
		}
	    }
	} else if (auto rep = left-> to <IMacroEnum> ()) {
	    if (auto var = right-> to <IVar> ()) {
		auto name = var-> token.getStr ();
		auto soluce = rep-> getSoluce ();
		auto elem = soluce.elements.find (name);
		if (elem != soluce.elements.end ())
		    return elem-> second-> templateExpReplace (values);		
	    }
	}
	
	return new (Z0)  IDot (this-> token, left, right);
    }

    Expression IExpand::templateExpReplace (const map <string, Expression>& values) {
	auto expr = this-> _expr-> templateExpReplace (values);
	return new (Z0)  IExpand (this-> token, expr);
    }

    Instruction IFor::templateReplace (const map <string, Expression>& values) {
	vector <Var> vars;
	for (auto it : this-> _var)
	    vars.push_back ((Var) it-> templateExpReplace (values));
	
	auto iter = this-> _iter-> templateExpReplace (values);

	if (this-> _isStatic && iter-> is <IMacroRepeat> () && vars.size () == 1) {
	    auto rep = iter-> to <IMacroRepeat> ();
	    auto bl = new (Z0) IBlock (this-> _block-> token, {}, {});
	    for (ulong i = 0 ; i < rep-> getSolution ().size () ; i++) {
		auto id = vars [0]-> token.getStr ();
		auto index = rep-> getSolution () [i];
		for (auto & it : index.elements)
		    it.second = it.second-> templateExpReplace (values);
		auto elem = new (Z0) IMacroRepeat (this-> _block-> token, NULL, NULL, false);
		elem-> addSolution (index);
		auto block = this-> _block-> templateExpReplace ({{id, elem}});
		if (block == NULL) return NULL;
		
		block = block-> templateExpReplace (values);
		if (block == NULL) return NULL;
		
		bl-> getInsts ().push_back (
		    block
		);
	    }
	    
	    return bl;
	}
	
	auto block = (Block) this-> _block-> templateReplace (values);
	auto ret = new (Z0)  IFor (this-> token, this-> _id, vars, iter, block, this-> _const);
	ret-> _isStatic = this-> _isStatic;
	return ret;
    }

    Expression IFuncPtr::templateExpReplace (const map <string, Expression>& values) {
	vector <Expression> params;
	for (auto it : this-> _params) {
	    auto elem = it-> templateExpReplace (values);
	    if (auto ps = elem-> to <IParamList> ()) {
		for (auto it_ : ps-> getParams ()) {
		    params.push_back (it_);
		}
	    } else {
		params.push_back (elem);
	    }
	}
	
	auto ret = this-> _ret-> templateExpReplace (values);
	// if (this-> expr) {
	//     auto expr = this-> expr-> templateExpReplace (values);
	//     return new (Z0)  IFuncPtr (this-> token, params, ret, expr);
	// } else
	return new (Z0)  IFuncPtr (this-> token, params, ret);
    }

    Declaration IFunction::templateDeclReplace (const map <string, Expression>& tmps) {
	return IFunction::templateReplace (tmps);
    }
  
    
    Declaration IModDecl::templateDeclReplace (const map <string, Expression> & tmps) {
	std::vector <Declaration> decls;
	for (auto it : this-> _decls) {
	    auto r = it-> templateDeclReplace (tmps);
	    if (r != NULL) 
		decls.push_back (r);
	}
	auto ret = new (Z0) IModDecl (this-> _ident, this-> getDocs (), decls);
	ret-> setPublic (this-> isPublic ());
	return ret;
    }
    
    Declaration IStruct::templateDeclReplace (const map <string, Expression> & tmps) {
	std::vector <Var> params;
	for (auto it : this-> params) {
	    params.push_back (it-> templateExpReplace (tmps)-> to<IVar> ());
	}
	
	auto ret = new (Z0) IStruct (this-> ident, this-> getDocs (), this-> _innerDocs, this-> tmps, params, this-> _udas);
	ret-> setPublic (this-> isPublic ());
	return ret;
    }
    
    Function IFunction::templateReplace (const map <string, Expression>& values) {
	Expression type = NULL;
	if (this-> _type)
	    type = this-> _type-> templateExpReplace (values);

	vector <Var> params;
	for (auto it : this-> _params)
	    params.push_back (it-> templateExpReplace (values)-> to <IVar> ());

	Expression test = NULL;
	if (this-> _test)
	    test = this-> _test-> templateExpReplace (values);

	vector <Expression> tmps;
	for (auto it : this-> _tmps) 
	    tmps.push_back (it-> templateExpReplace (values));

	auto block = (Block) this-> _block-> templateReplace (values);
	auto ret = new (Z0)  IFunction (this-> _ident, this-> getDocs (), this-> _attrs, type, this-> _retDeco, params, tmps, test, block);
	ret-> setPublic (this-> isPublic ());
	return ret;
    }

    Declaration ITypeCreator::templateDeclReplace (const map <string, Expression> & values) {
	Expression who;
	if (this-> _who) who = this-> _who-> templateExpReplace (values);

	std::vector <TypeConstructor> constr;
	for (auto it : this-> _constr) constr.push_back (it-> templateDeclReplace (values));

	std::vector <TypeDestructor> destr;
	for (auto it : this-> _destr) destr.push_back (it-> templateDeclReplace (values));

	std::vector <TypeMethod> methods;
	for (auto it : this-> _methods) methods.push_back (it-> templateDeclReplace (values));
	
	std::vector <TypeAttr> attrs;
	for (auto it : this-> _attrs) attrs.push_back (it-> templateDeclReplace (values));

	auto ret = new (Z0) ITypeCreator (this-> _ident, this-> getDocs (), who, {});
	ret-> getConstructors () = constr;
	ret-> getDestructors () = destr;
	ret-> getMethods () = methods;
	ret-> getAttrs () = attrs;
	
	return ret;
    }
    
    TypeConstructor ITypeConstructor::templateDeclReplace (const map<string, Expression> &tmps) {
	vector <Var> params;
	for (auto it : this-> _params)
	    params.push_back (it-> templateExpReplace (tmps)-> to <IVar> ());

	auto block = this-> _block-> templateReplace (tmps)-> to <IBlock> ();
	auto ret = new (Z0) ITypeConstructor (this-> _ident, params, block, this-> _isCopy);
	ret-> _prot = this-> _prot;
	return ret;
    }

    TypeDestructor ITypeDestructor::templateDeclReplace (const map <string, Expression> & tmps) {
	auto block = this-> _block-> templateReplace (tmps)-> to <IBlock> ();
	auto ret = new (Z0) ITypeDestructor (this-> _ident, block);
	ret-> _prot = this-> _prot;
	return ret;
    }

    TypeMethod ITypeMethod::templateDeclReplace (const map <string, Expression> & tmps) {
	auto ret = new (Z0) ITypeMethod (IFunction::templateDeclReplace (tmps)-> to <IFunction> (), this-> _isOver);
	ret-> _prot = this-> _prot;
	return ret;
    }

    TypeMethod ITypeMethod::templateReplace (const map<string, Expression> & tmps) {
	return this-> templateDeclReplace (tmps);
    }
    
    TypeAttr ITypeAttr::templateDeclReplace (const map <string, Expression> & tmps) {
	auto ret = new (Z0) ITypeAttr (this-> _ident, this-> getDocs (), this-> _type-> templateExpReplace (tmps), this-> _isConst, this-> _isStatic);
	ret-> _prot = this-> _prot;
	return ret;
    }
        
    Expression IIf::templateExpReplace (const map <string, Expression>& values) {
	Expression test = NULL;
	if (this-> _test)
	    test = this-> _test-> templateExpReplace (values);

	auto block = (Block) this-> _block-> templateReplace (values);
	If else_ = NULL;
	if (this-> _else)
	    else_ = (If) this-> _else-> templateExpReplace (values);
	
	return new (Z0)  IIf (this-> token, test, block, else_, this-> _isStatic);
    }

    Expression IIs::templateExpReplace (const map <string, Expression>& values) {
	auto left = this-> _left-> templateExpReplace (values);
	if (this-> _type) {
	    auto right = this-> _type-> templateExpReplace (values);
	    std::vector <Expression> tmps;
	    for (auto it : this-> _tmps)
		tmps.push_back (it-> templateExpReplace (values));
	    return new (Z0)  IIs (this-> token, left, right, tmps);
	} else {
	    return new (Z0)  IIs (this-> token, left, this-> _expType);
	}	    
    }

    Expression ILambdaFunc::templateExpReplace (const map <string, Expression>& values) {
	vector <Var> var;
	for (auto it : this-> _params)
	    var.push_back (it-> templateExpReplace (values)-> to <IVar> ());

	Var ret = NULL;
	if (this-> _ret)
	    ret = (Var) this-> _ret-> templateExpReplace (values);
	
	if (this-> _block) {
	    auto block = (Block) this-> _block-> templateReplace (values);
	    return new (Z0)  ILambdaFunc (this-> token, var, ret, block);
	} else if (this-> _expr) {
	    auto expr = this-> _expr-> templateExpReplace (values);
	    return new (Z0) ILambdaFunc (this-> token, var, expr);
	} else {
	    return new (Z0) ILambdaFunc (this-> token, this-> _frame);
	}
    }
    
    Expression IMatch::templateExpReplace (const map <string, Expression>& values) {
	auto expr = this-> _expr-> templateExpReplace (values);
	vector <Expression> auxValues;
	vector <Block> auxBlock;
	for (auto it : this-> _values)
	    auxValues.push_back (it-> templateExpReplace (values));

	for (auto it : this-> _block)
	    auxBlock.push_back ((Block) it-> templateReplace (values));

	return new (Z0)  IMatch (this-> token, expr, auxValues, auxBlock);
    }

    Expression IMixin::templateExpReplace (const map <string, Expression>& values) {
	auto inside = this-> inside-> templateExpReplace (values);
	return new (Z0)  IMixin (this-> token, inside);
    }

    Expression IOfVar::templateExpReplace (const map <string, Expression>& values) {
	auto type = this-> _type-> templateExpReplace (values);
	return new (Z0)  IOfVar (this-> token, type);
    }

    Expression IVariadicVar::templateExpReplace (const map <string, Expression>&) {
	return new (Z0) IVariadicVar (this-> token);
    }

    Expression IParamList::templateExpReplace (const map <string, Expression>& values) {
	vector <Expression> params;
	for (auto it : this-> _params) {
	    auto elem = it-> templateExpReplace (values);
	    if (elem == NULL) return NULL;
	    if (auto ps = elem-> to <IParamList> ()) {
		for (auto it_ : ps-> getParams ()) {
		    params.push_back (it_-> to <IVar> ());
		}
	    } else {
		params.push_back (elem);
	    }
	}
	return new (Z0)  IParamList (this-> token, params);
    }

    Expression IPar::templateExpReplace (const map <string, Expression>& values) {
	auto params = (ParamList) this-> _params-> templateExpReplace (values);
	auto left = this-> _left-> templateExpReplace (values);
	if (params == NULL || left == NULL) return NULL;
	return new (Z0)  IPar (this-> token, this-> _end, left, params);
    }

    Instruction IReturn::templateReplace (const map <string, Expression>& values) {
	if (this-> _elem == NULL) return new (Z0)  IReturn (this-> token);
	auto ret = new (Z0)  IReturn (this-> token, this-> _elem-> templateExpReplace (values));
	ret-> isUseless () = this-> isUseless ();
	return ret;	
    }

    Instruction ITupleDest::templateReplace (const map <string, Expression>& values) {
	vector <Var> decls;
	for (auto it : this-> decls)
	    decls.push_back ((Var) it-> templateExpReplace ({}));

	auto right = this-> right-> templateExpReplace (values);
	return new (Z0)  ITupleDest (this-> token, this-> isVariadic, decls, right);
    }
    
    Expression IConstTuple::templateExpReplace (const map <string, Expression>& values) {
	vector <Expression> exprs;
	for (auto it : this-> params)
	    exprs.push_back (it-> templateExpReplace (values));

	auto ret = new (Z0)  IConstTuple (this-> token, this-> end, exprs);
	ret-> isFake () = this-> isFake ();
	return ret;
    }

    Declaration IGlobal::templateDeclReplace (const map <string, Expression>& values) {
	if (this-> _expr) {
	    return new (Z0) IGlobal (this-> _ident, this-> getDocs (), this-> _expr-> templateExpReplace (values));
	} else {	    
	    return new (Z0) IGlobal (this-> _ident, this-> getDocs (), this-> _type-> templateExpReplace (values), this-> _isExternal);
	}
    }

    Expression ITypedVar::templateExpReplace (const map <string, Expression>& values) {
	Expression type;
	auto elem = this-> type-> templateExpReplace (values);
	if (auto ps = elem-> to <IParamList> ()) {
	    type = paramListToTupleVar (this-> type-> token, ps);
	} else {
	    type = elem;
	}
	return new (Z0)  ITypedVar (this-> token, type, this-> deco);

    }

    Expression IType::templateExpReplace (const map <string, Expression>&) {
	if (this-> _type == NULL) return NULL;
	return new (Z0)  IType (this-> token, this-> _type);
    }
    
    Expression ITypeOf::templateExpReplace (const map <string, Expression>& values) {
	auto left = this-> expr-> templateExpReplace (values);
	return new (Z0)  ITypeOf (this-> token, left, this-> _mut);
    }

    Expression IStringOf::templateExpReplace (const map <string, Expression>& values) {
	auto left = this-> expr-> templateExpReplace (values);
	return new (Z0) IStringOf (this-> token, left);
    }
    
    Expression IUnary::templateExpReplace (const map <string, Expression>& values) {
	auto elem = this-> elem-> templateExpReplace (values);
	return new (Z0)  IUnary (this-> token, elem);
    }

    Instruction IVarDecl::templateReplace (const map <string, Expression>& values) {
	vector <Expression> insts;
	vector <Var> decls;
	vector <Word> decos;
	for (auto it : this-> decls)
	    decls.push_back (it-> templateExpReplace ({values})-> to <IVar> ());

	for (auto it : this-> decos)
	    decos.push_back (it);

	for (auto it : this-> insts) {
	    if (it)
		insts.push_back (it-> templateExpReplace (values));
	    else insts.push_back (NULL);
	}
	
	return new (Z0)  IVarDecl (this-> token, decos, decls, insts);
    }

    Expression IVar::templateExpReplace (const map <string, Expression>& values) {
	Var ret = NULL;
	for (auto it : values) {
	    if (it.first == this-> token.getStr ()) {
		auto clo = it.second-> templateExpReplace ({});
		if (clo == NULL) return NULL;
		clo-> token.setLocus (this-> token.getLocus ());
		if (auto t = clo-> to <IType> ()) {
		    t-> deco = this-> deco;
		    if (this-> deco == Keys::REF && !clo-> info-> type ()-> is <semantic::IRefInfo> ()) {
			t-> info-> type (new semantic::IRefInfo (t-> info-> isConst (), t-> info-> type ()));
		    }
		    ret = t;
		    break;
		} else if (auto v = clo-> to <IVar> ()) {
		    v-> deco = this-> deco;
		    ret = v;
		    break;		
		} else if (this-> templates.size () == 0) {
		    return clo;
		} else {
		    break;
		}
	    }
	}
	
	vector <Expression> tmps;
	if (this-> token == Keys::CONST && this-> templates.size () == 1) {
	    auto elem = this-> templates [0]-> templateExpReplace (values);
	    if (auto ps = elem-> to <IParamList> ()) {
		auto type = paramListToTupleVar (elem-> token, ps);
		tmps.push_back (type);
	    } else tmps.push_back (elem);						 
	} else {
	    for (auto it : this-> templates) {
		auto elem = it-> templateExpReplace (values);
		if (elem == NULL) return NULL;
		if (auto ps = elem-> to<IParamList> ()) {
		    for (auto it_ : ps-> getParams ()) {
			tmps.push_back (it_);
		    }
		} else {
		    tmps.push_back (elem);
		}
	    }
	}

	if (ret == NULL)
	    ret = new (Z0)  IVar (this-> token, tmps);
	else ret-> templates = tmps;
	ret-> deco = this-> deco;
	return ret;	
    }
    
    Expression IArrayVar::templateExpReplace (const map <string, Expression>& values) {
	auto cont = this-> content-> templateExpReplace (values);
	if (auto ps = cont-> to <IParamList> ()) {
	    cont = ps;//paramListToTupleVar (this-> content-> token, ps);
	}
	
	auto ret = new (Z0)  IArrayVar (this-> token, cont);
	if (this-> len) {
	    ret-> len = this-> len-> templateExpReplace (values);
	}
	ret-> deco = this-> deco;
	return ret;
    }

    Instruction IWhile::templateReplace (const map <string, Expression>& values) {
	auto test = this-> test-> templateExpReplace (values);
	auto block = (Block) this-> block-> templateReplace (values);
	return new (Z0)  IWhile (this-> token, test, block);
    }

    
    Declaration IUse::templateDeclReplace (const map <string, Expression> & values) {
	auto mod = this-> mod-> templateExpReplace (values);
	auto ret = new (Z0) IUse (this-> loc, mod);
	ret-> setPublic (this-> isPublic ());
	return ret;
    }

    Expression IPragma::templateExpReplace (const map <string, Expression> & values) {
	return new (Z0) IPragma (this-> token, this-> _params-> templateReplace (values)-> to <IParamList> ());
    }

    Declaration IImport::templateDeclReplace (const map <string, Expression> &) {
	return new (Z0) IImport (this-> _ident, this-> _params);
    }
       
    Expression IMacroCall::templateExpReplace (const std::map <std::string, Expression>& values) {
	auto left = this-> _left-> templateExpReplace (values);
	
	std::vector <Word> content;
	lexical::FakeLexer lex {this-> _content};
	lex.skipEnable ({Token::SPACE, Token::RETURN, Token::RRETURN, Token::TAB} , false);
	
	while (true) {
	    auto next = lex.next ();
	    if (next.isEof ()) break;
	    if (next == Token::DOLLAR) {
		lex.skipEnable ({Token::SPACE, Token::RETURN, Token::RRETURN, Token::TAB} , true);
		lex.next ({Token::LPAR});
		syntax::Visitor visit (lex);		
		auto expr = visit.visitExpression ();		
		lex.next ({Token::RPAR});
		lex.skipEnable ({Token::SPACE, Token::RETURN, Token::RRETURN, Token::TAB} , false);
		expr = expr-> templateExpReplace (values);
		if (!expr) return NULL;
		if (auto elem = expr-> to<IMacroElement> ()) {
		    bool success = false;
		    auto current = elem-> toTokens (success);
		    if (!success) return NULL;
		    content.insert (content.end (), current.begin (), current.end ());
		} else content.push_back (expr-> token);		
	    } else content.push_back (next);
	}
	
	auto aux = new (Z0) IMacroCall (this-> token, this-> _end, left, content);	
	return aux-> solve (values);
    }

    Expression IMacroVar::templateExpReplace (const std::map <std::string, Expression>&) {
	return this-> _content-> templateExpReplace ({});
    }

    Expression IMacroRepeat::templateExpReplace (const std::map <std::string, Expression> & values) {
	auto res = new (Z0) IMacroRepeat (this-> token, this-> _content, this-> _pass, this-> _oneTime);
	for (auto soluce : this-> _soluce) {
	    std::map <std::string, Expression> current;
	    for (auto it : soluce.elements) {
		current [it.first] = it.second-> templateExpReplace (values);
	    }
	    res-> addSolution ({true, current, NULL});
	}
	return res;
    }

    Expression IMacroEnum::templateExpReplace (const std::map <std::string, Expression> & values) {
	auto res = new (Z0) IMacroEnum (this-> token, this-> _elems);
	std::map <std::string, Expression> current;
	for (auto it : this-> _soluce.elements) {
	    current [it.first] = it.second-> templateExpReplace (values);
	}
	res-> _soluce = {true, current, NULL};
	return res;
    }
    
    Expression IMacroToken::templateExpReplace (const std::map <std::string, Expression> &) {
	return new (Z0) IMacroToken (this-> token, this-> _value);
    }

    Expression IEvaluatedExpr::templateExpReplace (const std::map <std::string, Expression> &) {
	return new (Z0) IEvaluatedExpr (this-> _value);
    }
    
    
}
