#include <ymir/ast/_.hh>
#include <ymir/semantic/pack/_.hh>
#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/value/_.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/semantic/tree/Generic.hh>

namespace syntax {

    using namespace semantic;

    std::vector <semantic::Symbol> INone::allInnerDecls () {
	return {};
    }
    
    Instruction IBlock::instruction () {
	return this-> block ();
    }

    Block IBlock::block () {
	Table::instance ().enterBlock ();
	auto ret = blockWithoutEnter ();
	Table::instance ().quitBlock ();
	return ret;
    }

    Expression IBlock::getLastExpr () {
	if (this-> _insts.size () != 0) {
	    return this-> _insts.back ()-> to <IExpression> ();
	}
	return NULL;
    }

    std::vector <Symbol> IBlock::allInnerDecls () {
	std::vector <Symbol> allInner;
	for (auto it : this-> _insts) {
	    auto decls = it-> allInnerDecls ();
	    allInner.insert (allInner.end (), decls.begin (), decls.end ());
	}
	return allInner;
    }
    
    Block IBlock::blockWithoutEnter () {
	std::vector <Instruction> insts;
	auto bl = new (Z0)  IBlock (this-> token, {}, {});
	
	if (!this-> _ident.isEof ()) 
	    Table::instance ().retInfo ().setIdent (this-> _ident);

	for (auto it : this-> _decls) {
	    it-> declareAsInternal ();
	}

	for (auto it : this-> _insts) {
	    if (Table::instance ().retInfo ().hasReturned () ||
		Table::instance ().retInfo ().hasBreaked ()) {
		auto fail = true;
		if (auto ret = it-> to <IReturn> ()) {
		    if (ret-> isUseless ()) fail = false;
		}
		if (fail) {
		    if (it-> is<IScope> ()) {
			Ymir::Error::scopeExitEnd (it-> token);
		    } else {
			Ymir::Error::unreachableStmt (it-> token);
			break;
		    }
		}
	    }
	    if (!it-> is<INone> ()) {
		it-> father () = bl;
		auto inst = it-> instruction ();
		if (inst != NULL) {
		    insts.push_back (inst);
		    insts.back ()-> father () = bl;
		}
	    }	    	    
	}

	for (auto it : this-> _preFinally) {
	    auto ret = it-> instruction ();
	    if (ret != NULL) {
		ret-> father () = bl;
		bl-> addFinally (new (Z0) IBlock (this-> _ident, {}, {ret}));
	    }
	}
	
	bl-> _insts = insts;
	return bl;    
    }
    
    std::vector <semantic::Symbol> IVarDecl::allInnerDecls () {
	std::vector <Symbol> syms;
	for (auto it : this-> decls) {
	    syms.push_back (it-> info);
	}
	return syms;
    }

    Instruction IVarDecl::instruction () {
	auto auxDecl = new (Z0)  IVarDecl (this-> token);
	for (auto id : Ymir::r (0, this-> decls.size ())) {
	    auto it = this-> decls [id];
	    if (!it || !it-> is<IVar> ()) {
		Ymir::Error::noLet (this-> token);
		continue;
	    } 
	    auto aux = new (Z0)  IVar (it-> token);
	    auto info = Table::instance ().get (it-> token.getStr ());
	    if (info && Table::instance ().sameFrame (info)) {
		Ymir::Error::shadowingVar (it-> token, info-> sym);
		return NULL;
	    }

	    if (this-> decos.size () > (uint) id && this-> decos [id] == Keys::IMMUTABLE) {
		if (auto bin = this-> insts [id]-> to<IBinary> ()) {
		    auto type = bin-> getRight ()-> expression ();
		    if (type == NULL) return NULL;
		    aux-> info = new (Z0) ISymbol (aux-> token, aux, type-> info-> type ()-> clone ());
		    aux-> info-> isConst (true);
		    aux-> info-> value () = type-> info-> type ()-> value ();
		    if (!aux-> info-> isImmutable ()) {
			Ymir::Error::notImmutable (this-> token, type-> info);
			return NULL;
		    }
		    Table::instance ().insert (aux-> info);
		    auxDecl-> decls.push_back (aux);
		    auxDecl-> insts.push_back (NULL);
		} else {
		    Ymir::Error::immutNoInit (it-> token);
		    return NULL;
		}		    
	    } else if (this-> decos.size () > (uint) id && this-> decos [id] == Keys::CONST) {
		aux-> info = new (Z0)  ISymbol (aux-> token, aux, new (Z0)  IUndefInfo ());
		aux-> info-> isConst (true);
		Table::instance ().insert (aux-> info);
		if (this-> insts [id] == NULL) {
		    Ymir::Error::constNoInit (it-> token);
		    return NULL;
		} else {
		    auxDecl-> decls.push_back (aux);
		    auxDecl-> insts.push_back (this-> insts [id]-> expression ());
		    aux-> info-> isConst (true);
		    aux-> info-> value () = NULL;
		}
	    } else if (this-> decos.size () > (uint) id && this-> decos [id] == Keys::STATIC) {
		auto space = Table::instance ().space ();
		if (auto bin = this-> insts [id]-> to<IBinary> ()) {
		    auto type = bin-> getRight ()-> expression ();
		    aux-> info = new (Z0) ISymbol (aux-> token, aux, type-> info-> type ()-> clone ());
		    aux-> info-> isConst (false);
		    aux-> info-> value () = NULL;
		    Table::instance ().insert (aux-> info);
		    auxDecl-> statics.push_back (aux);
		    auxDecl-> staticExprs.push_back (type);
		} else {
		    Ymir::Error::staticNoInit (it-> token);
		    return NULL;
		}		
	    } else {
		aux-> info = new (Z0)  ISymbol (aux-> token, aux, new (Z0)  IUndefInfo ());
		aux-> info-> isConst (false);
		Table::instance ().insert (aux-> info);
		auxDecl-> decls.push_back (aux);
		if (this-> insts [id]) 
		    auxDecl-> insts.push_back (this-> insts [id]-> expression ());
		else auxDecl-> insts.push_back (NULL);
		aux-> info-> value () = NULL;
	    }
	}
	
	return auxDecl;
    }

    Instruction IAssert::instruction () {
	auto expr = this-> _expr-> expression ();
	if (expr == NULL) return NULL;
	if (!expr-> info-> type ()-> isSame (new (Z0)  IBoolInfo (true))) {
	    Ymir::Error::incompatibleTypes (this-> token, expr-> info, new (Z0)  IBoolInfo (true));
	    return NULL;
	}
	
	Expression msg = NULL;
	if (this-> _msg) {
	    msg = this-> _msg-> expression ();
	    if (msg == NULL) return NULL;
	    if (!msg-> info-> type ()-> isSame (new (Z0) IStringInfo (true))) {
		Ymir::Error::incompatibleTypes (this-> token, expr-> info, new (Z0) IStringInfo (true));
		return NULL;
	    }
	}

	if (this-> _isStatic) {
	    if (msg && !msg-> info-> isImmutable ()) {
		Ymir::Error::notImmutable (this-> token, msg-> info);
	    }

	    if (!expr-> info-> isImmutable ()) {
		Ymir::Error::notImmutable (this-> token, expr-> info);
	    } else if (!(expr-> info-> value ()-> to<IBoolValue> ()-> isTrue ())) {
		if (msg) 
		    Ymir::Error::assertFailure (this-> token, msg-> info-> value ()-> toString ().c_str ());
		else Ymir::Error::assertFailure (this-> token);
	    }
	} else if (expr-> info-> isImmutable ()) {
	    if (!expr-> info-> value ()-> to<IBoolValue> ()-> isTrue ())
		Table::instance ().retInfo ().returned ();
	}
	return new (Z0)  IAssert (this-> token, expr, msg, this-> _isStatic);
    }
    
    Instruction IThrow::instruction () {
	if (Table::instance ().hasCurrentContext (Keys::SAFE)) {
	    Ymir::Error::throwInSafe (this-> token);
	}
	
	auto expr = this-> _expr-> expression ();
	if (expr == NULL) return NULL;

	auto type = expr-> info-> type ()-> CompOp (new (Z0) IUndefInfo ());
	if (type == NULL) {
	    Ymir::Error::incompatibleTypes (this-> token, expr-> info, new (Z0) IUndefInfo ());
	    return NULL;
	}

	if (auto func = expr-> info-> type ()-> to <IPtrFuncInfo> ()) {
	    if (func-> isDelegate ()) {
		if (!Table::instance ().verifyClosureLifeTime (0, func-> closures ())) {
		    Ymir::Error::here (this-> token);
		}
	    }
	}
	
	auto aux = new (Z0) IThrow (this-> token, expr);
	aux-> _caster = type-> cloneOnExitWithInfo ();
	return aux;
    }

    std::vector <semantic::Symbol> IAssert::allInnerDecls () {
	return {};
    }
    
    std::vector <semantic::Symbol> IThrow::allInnerDecls () {
	return {};
    }

    std::vector <semantic::Symbol> IBreak::allInnerDecls () {
	return {};
    }

    Instruction IBreak::instruction () {
	auto aux = new (Z0)  IBreak (this-> token, this-> _ident);
	Table::instance ().retInfo ().breaked ();
	if (this-> _ident.isEof ()) {
	    auto nb = Table::instance ().retInfo ().rewind (std::vector <std::string> {"while", "for"});
	    if (nb == -1) {
		Ymir::Error::breakOutSide (this-> token);
		return NULL;
	    } else {
		aux-> _nbBlock = nb;
	    }
	} else {
	    auto nb = Table::instance ().retInfo ().rewind (this-> _ident.getStr ());
	    if (nb == -1) {
		Ymir::Error::breakRefUndef (this-> _ident);
		return NULL;
	    } else aux-> _nbBlock = nb;
	}
	return aux;
    }    

    std::vector <semantic::Symbol> IIf::allInnerDecls () {
	auto ret = this-> _block-> allInnerDecls ();
	auto el = this-> _else-> allInnerDecls ();
	ret.insert (ret.begin (), el.begin (), el.end ());
	return ret;
    }
    
    Instruction IIf::instruction () {
	if (this-> _test != NULL) {
	    auto expr = this-> _test-> expression ();
	    if (expr == NULL) return NULL;
	    auto type = expr-> info-> type ()-> CompOp (new (Z0)  IBoolInfo (true));
	    
	    if (type == NULL) {
		Ymir::Error::incompatibleTypes (this-> token, expr-> info, new (Z0)  IBoolInfo (true));
		return NULL;
	    }


	    if (this-> _isStatic) {
		if (!expr-> info-> isImmutable ()) {
		    Ymir::Error::notImmutable (this-> token, expr-> info);
		    return NULL;
		} else if (expr-> info-> value ()-> to<IBoolValue> ()-> isTrue ()) {
		    return this-> _block-> instruction ();
		} else {
		    if (this-> _else) {
			this-> _else-> _isStatic  = this-> _isStatic;
			return this-> _else-> instruction ();
		    }
		    else return new (Z0)  IBlock (this-> _block-> token, {}, {});
		}
	    }
	    
	    Table::instance ().retInfo ().currentBlock () = "if";
	    Table::instance ().retInfo ().changed () = true;
	    Block bl = this-> _block-> block ();
	    If if_;
	    if (this-> _else != NULL) {
		if_ = new (Z0)  IIf (this-> token, expr, bl, (If) this-> _else-> instruction ());
		if_-> _info = type;
	    } else {
		if_ = new (Z0)  IIf (this-> token, expr, bl);
		if_-> _info = type;
	    }
	    return if_;
	} else {
	    if (!this-> _isStatic) {
		Table::instance ().retInfo ().currentBlock () = "else";
		Table::instance ().retInfo ().changed () = true;
	    
		Block bl = this-> _block-> block ();
		If if_ = new (Z0)  IIf (this-> token, NULL, bl);
		return if_;
	    } else {
		return this-> _block-> instruction ();
	    }
	}
    }

    std::vector <semantic::Symbol> IFor::allInnerDecls () {
	std::vector <Symbol> syms;
	for (auto it : this-> _var)
	    syms.push_back (it-> info);
	auto bl = this-> _block-> allInnerDecls ();
	syms.insert (syms.end (), bl.begin (), bl.end ());
	return syms;
    }
    
    Instruction IFor::immutable (Expression expr) {
	if (expr-> info-> type ()-> is<IRangeInfo> () && this-> _var.size () == 1) {
	    return immutableRange (this-> _var, expr);
	} else if (expr-> info-> type ()-> is <ITupleInfo> () && this-> _var.size () == 1) {
	    return immutableTuple (this-> _var, expr);
	} else if (expr-> info-> type ()-> is <IMacroRepeatInfo> () && this-> _var.size () == 1) {
	    return immutableMacro (this-> _var, expr);
	}
	
	Ymir::Error::undefinedOp (this-> token, expr-> info);
	return NULL;
    }

    Instruction IFor::immutableTupleFake (std::vector <Var> & vars, Expression expr) {
	auto tu = expr-> info-> type ()-> to <ITupleInfo> ();
	auto ctuple = expr-> to <IConstTuple> ();
	Table::instance ().enterBlock ();
	Block bl = new (Z0) IBlock (this-> token, {}, {});
	for (auto it : Ymir::r (0, tu-> nbParams ())) {
	    std::map<std::string, Expression> res = {{vars [0]-> token.getStr (), tu-> getParams ()[it]-> value ()-> toYmir (ctuple-> getExprs ()[it]-> info)}};
	    auto currentBl = this-> _block-> templateReplace (res)-> to<IBlock> ();
	    bl-> getInsts ().push_back (currentBl-> block ());
	}
	Table::instance ().quitBlock ();
	return bl;
    }
    
    Instruction IFor::immutableTuple (std::vector <Var> & vars, Expression expr) {
	auto tu = expr-> info-> type ()-> to <ITupleInfo> ();
	if (tu-> isFake ()) return immutableTupleFake (vars, expr);
	
	Table::instance ().enterBlock ();
	Block bl = new (Z0) IBlock (this-> token, {}, {});
	auto varDecl = new IVarDecl (this-> token, {}, {}, {});
	bl-> getInsts ().push_back (varDecl);
	expr = expr-> expression ();

	auto name = Ymir::OutBuffer ("_", ISymbol::getLastTmp ()).str () ;
	auto aux = new (Z0) IVar ({{this-> token, name} , name});	
	aux-> info = new (Z0) ISymbol (aux-> token, aux, new (Z0) IRefInfo (true, expr-> info-> type ()-> clone ()));
	Table::instance ().insert (aux-> info);
	varDecl-> getDecls ().push_back (aux);
	varDecl-> getInsts ().push_back ((new (Z0) IAffectGeneric (this-> token,
								   aux, expr,
								   true
	))-> expression ()
	);
	
	for (auto i : Ymir::r (0, tu-> nbParams ())) {
	    auto var = vars [0]-> templateExpReplace ({})-> to <IVar> ();
	    Table::instance ().enterBlock ();
	    auto index = new (Z0) IFixedInfo (true, FixedConst::UINT);
	    index-> value () = new (Z0) IFixedValue (FixedConst::UINT, i, i);
	    auto indexVal = index-> value ()-> toYmir (new (Z0) ISymbol (var-> token, var, index));
	    auto currentVal = (new (Z0) IDot (this-> token, aux, indexVal))-> expression ();

	    var-> info = new (Z0) ISymbol (var-> token, var, new (Z0) IRefInfo (this-> _const [0],
										this-> _const [0] ?
										currentVal-> info-> type ()-> cloneConst () : currentVal-> info-> type ()-> clone ())
	    );
	    
	    Table::instance ().insert (var-> info);
	    varDecl-> getDecls ().push_back (var);
	    varDecl-> getInsts ().push_back (NULL);
	    
	    bl-> getInsts ().push_back ((new (Z0) IAffectGeneric ({this-> token, Token::EQUAL},
								  var,
								  currentVal,
								  true
	    ))-> expression ());
	    
	    bl-> getInsts ().push_back (this-> _block-> block ());
	    Table::instance ().quitBlock ();
	}
	Table::instance ().quitBlock ();
	return bl;
    }
    
    Instruction IFor::immutableRange (std::vector <Var> & vars, Expression expr) {
	auto range = expr-> info-> type ()-> to <IRangeInfo> ();
	if (!range-> leftValue () || !range-> rightValue ()) {
	    Ymir::Error::notImmutable (this-> token, expr-> info);
	    return NULL;
	}

	Block bl = new (Z0) IBlock (this-> token, {}, {});
	if (auto li = range-> leftValue ()-> to <IFixedValue> ()) {
	    auto ri = range-> rightValue ()-> to <IFixedValue> ();
	    if (isSigned (li-> getType ())) {
		ulong left = li-> getUValue (), right = ri-> getUValue ();
		for (ulong i = left ; i != right ; i += left > right ? -1 : 1) {
		    auto var = vars [0]-> templateExpReplace ({});
		    auto index = range-> content ()-> cloneConst ();
		    index-> value () = new (Z0) IFixedValue (li-> getType (), i, i);
		    var-> info = new (Z0) ISymbol (var-> token, var, index);
		    Table::instance ().enterBlock ();
		    Table::instance ().insert (var-> info);
		    bl-> getInsts ().push_back (this-> _block-> block ());
		    Table::instance ().quitBlock ();
		}
	    } else {
		long left = li-> getValue (), right = ri-> getValue ();
		for (long i = left ; i != right ; i += left > right ? -1 : 1) {
		    auto var = vars [0]-> templateExpReplace ({});
		    auto index = range-> content ()-> cloneConst ();
		    index-> value () = new (Z0) IFixedValue (li-> getType (), i, i);
		    var-> info = new (Z0) ISymbol (var-> token, var, index);
		    Table::instance ().enterBlock ();
		    Table::instance ().insert (var-> info);
		    bl-> getInsts ().push_back (this-> _block-> block ());
		    Table::instance ().quitBlock ();
		}
	    }
	} else {
	    Ymir::Error::notImmutable (this-> token, expr-> info);
	    return NULL;
	}
	
	return bl;
    }

    Instruction IFor::immutableMacro (std::vector <Var> & vars, Expression expr) {
	auto rep = expr-> info-> type ()-> to <IMacroRepeatInfo> ();
	Block bl = new (Z0) IBlock (this-> token, {}, {});
	for (ulong i = 0 ; i < rep-> getLen () ; i++) {
	    auto var = vars [0]-> templateExpReplace ({});
	    auto index = rep-> getValue (i);
	    var-> info = new (Z0) ISymbol (var-> token, var, index);
	    Table::instance ().enterBlock ();
	    Table::instance ().insert (var-> info);
	    bl-> getInsts ().push_back (this-> _block-> block ());
	    Table::instance ().quitBlock ();
	}
	return bl;
    }
    
    Instruction IFor::instruction () {
	auto expr = this-> _iter-> expression ();
	if (expr == NULL) return NULL;
	if (!this-> _id.isEof () && !this-> _isStatic)
	    Table::instance ().retInfo ().setIdent (this-> _id);
	else if (!this-> _id.isEof () && this-> _isStatic) {
	    Ymir::Error::labelingImmutableFor (this-> _id);
	}
	
	if (!this-> _isStatic) {
	    Table::instance ().enterBlock ();
	    std::vector <Var> var (this-> _var.size ());
	    for (auto it : Ymir::r (0, this-> _var.size ())) {
		var [it] = (Var) this-> _var [it]-> templateExpReplace ({});
		auto info = Table::instance ().get (var [it]-> token.getStr ());
		if (info && Table::instance ().sameFrame (info)) {
		    Ymir::Error::shadowingVar (var [it]-> token, info-> sym);
		    return NULL;
		}
		
		var [it]-> info = new (Z0) ISymbol (var [it]-> token, var [it], new (Z0) IUndefInfo ());
		var [it]-> info-> isConst (false);
		var [it]-> info-> value () = NULL;
		if (!this-> _isStatic)
		    Table::instance ().insert (var [it]-> info);
	    }
	
	    auto type = expr-> info-> type ()-> ApplyOp (var);
	    if (type == NULL) {
		Table::instance ().quitBlock ();
		auto call = findOpApply ();
		if (call == NULL) {
		    Ymir::Error::undefinedOp (this-> token, expr-> info);
		    return NULL;
		} else return call;
	    }

	    Table::instance ().retInfo ().currentBlock () = "for";
	    Table::instance ().retInfo ().changed () = true;
	    for (auto it : Ymir::r (0, this-> _var.size ())) {
		var [it]-> info-> type ()-> isConst (this-> _const [it]);
	    }
	    
	    Block bl = this-> _block-> block ();
	    Table::instance ().quitBlock ();
	    auto aux = new (Z0) IFor (this-> token, this-> _id, var, expr, bl, this-> _const);
	    aux-> _ret = type;
	    return aux;
	} else {
	    return this-> immutable (expr);	    
	}
    }

    Instruction IFor::findOpApply () {
	Word beginW {this-> token, Keys::OPAPPLYBEGIN};
	Word nextW {this-> token, Keys::OPAPPLYITER};
	Word endW {this-> token, Keys::OPAPPLYEND};
	Word getW {this-> token, Keys::OPAPPLYGET};
	
	auto block = new (Z0) IBlock (this-> token,  {}, {});
	auto nbVar = new (Z0) IFixed (this-> token, FixedConst::INT);
	nbVar-> setValue ((int) this-> _var.size ());
	auto params = new (Z0) IParamList (this-> token, {this-> _iter});
	Word tok {this-> token, Token::LPAR}, tok2 {this-> token, Token::RPAR};
	auto begin = new (Z0) IPar (tok, tok2, new (Z0) IVar (beginW, {nbVar}), params);

	auto decl = new (Z0) IVarDecl (this-> token, {}, {}, {});
	auto iterator = new (Z0) IVar ({this-> token, Ymir::OutBuffer ("#", this-> _var [0]-> token.getStr ()).str ()});

	decl-> getDecls ().push_back (iterator);
	decl-> getInsts ().push_back (new (Z0) IBinary ({this-> token, Token::EQUAL},
							iterator, begin));

	block-> getInsts ().push_back (decl);	
	auto innerBlock = new (Z0) IBlock (this-> token, {}, {});
	for (auto it : Ymir::r (0, this-> _var.size ())) {
	    auto var = this-> _var [it]-> templateExpReplace ({})-> to <IVar> ();
	    auto val = new (Z0) IFixed (this-> token, FixedConst::INT);
	    val-> setValue ((int) it);
	    auto right = new (Z0) IPar (tok, tok2, new (Z0) IVar (getW, {val}), new (Z0) IParamList (this-> token, {iterator}));
	    innerBlock-> getInsts ().push_back (new (Z0) IFakeDecl (this-> token, var, right, this-> _const [it], true));
	}
	
	innerBlock-> getInsts ().push_back (this-> _block-> templateExpReplace ({}));	
	innerBlock-> getInsts ().push_back (new (Z0) IPar (tok, tok2, new (Z0) IVar (nextW), new (Z0) IParamList (this-> token, {iterator})));	
	block-> getInsts ().push_back (new (Z0) IWhile (this-> token,
							new (Z0) IUnary ({this-> token, Token::NOT},
									 new (Z0) IPar (tok, tok2, new (Z0) IVar (endW), new (Z0) IParamList (this-> token, {iterator}))
							),
							innerBlock));
	
	return block-> expression ();
    }
    
    std::vector <semantic::Symbol> IWhile::allInnerDecls () {
	return this-> block-> allInnerDecls ();
    }
    
    Instruction IWhile::instruction () {
	auto expr = this-> test-> expression ();
	if (expr == NULL) return NULL;
	auto type = expr-> info-> type ()-> CompOp (new (Z0)  IBoolInfo (true));

	if (type == NULL) {
	    Ymir::Error::incompatibleTypes (this-> token, expr-> info, new (Z0)  IBoolInfo (true));
	}
	Table::instance ().enterBlock ();
	if (!this-> name.isEof ())
	    Table::instance ().retInfo ().setIdent (this-> name);
	
	Table::instance ().retInfo ().currentBlock () = "while";
	Table::instance ().retInfo ().changed () = true;
	Block bl = this-> block-> block ();
	Table::instance ().quitBlock ();
	
	return new (Z0)  IWhile (this-> token, this-> name, expr, bl);		
    }

    std::vector <semantic::Symbol> IReturn::allInnerDecls () {
	return {};
    }    

    void IReturn::verifLocal () {
	if (this-> _elem && this-> _elem-> is <IUnary> ()) {
	    auto un = this-> _elem-> to <IUnary> ();
	    if (un-> token == Token::AND) {
		auto sym = un-> getElem ();
		if (!sym-> info-> type ()-> is<IRefInfo> ()) {
		    if (Table::instance ().isFrameLocal (sym-> info)) {
			Ymir::Error::returnLocalAddr (sym-> info-> sym, this-> token);
		    }
		}
	    }
	}
	
	if (this-> _elem && this-> _elem-> info && this-> _elem-> info-> type ()-> is <IPtrFuncInfo> ()) {
	    auto func = this-> _elem-> info-> type ()-> to<IPtrFuncInfo> ();
	    if (func-> isDelegate ()) {
		if (!Table::instance ().verifyClosureLifeTime (0, func-> closures ())) {
		    Ymir::Error::here (this-> token);
		}
	    }
	}
    }
    
    Instruction IReturn::instruction () {
	auto aux = new (Z0)  IReturn (this-> token);
	Table::instance ().retInfo ().returned ();
	if (this-> _elem != NULL) {
	    this-> _elem-> inside = this;
	    aux-> _elem = this-> _elem-> expression ();
	    if (aux-> _elem == NULL) return NULL;
	    if (aux-> _elem-> info-> type ()-> is <IVoidInfo> ()) {
		Ymir::Error::returnVoid (this-> token, aux-> _elem-> info);
		return NULL;
	    }
	    
	    auto type = aux-> _elem-> info-> type ()-> CompOp (Table::instance ().retInfo ().info-> type ());	    
	    if (type == NULL) {		
		Ymir::Error::incompatibleTypes (this-> token, aux-> _elem-> info, Table::instance ().retInfo ().info-> type ());
		return NULL;
	    }
	    
	    if (Table::instance ().retInfo ().info-> type ()-> is <IUndefInfo> ())
		Table::instance ().retInfo ().info-> type (type);
	    
	    if (type-> isSame (aux-> _elem-> info-> type ())) {		
		if (!Table::instance ().retInfo ().changed ()) {
		    Table::instance ().retInfo ().info-> value () =
			aux-> _elem-> info-> value ();
		} else
		    Table::instance ().retInfo ().info-> value () = NULL;
		Table::instance ().retInfo ().changed () = true;
	    }
	    	    	    
	    aux-> _caster = type-> cloneOnExitWithInfo ();
	    if (Table::instance ().retInfo ().deco == Keys::CONST)
	    	aux-> _caster-> isConst (true);

	    if (!aux-> _caster-> ConstVerif (Table::instance ().retInfo ().info-> type ())) {
		Ymir::Error::incompatibleTypes (this-> token, aux-> _elem-> info, Table::instance ().retInfo ().info-> type ());
		return NULL;		    
	    }	    
	} else {
	    if (!Table::instance ().retInfo ().info-> type ()-> is<IUndefInfo> () &&
		!Table::instance ().retInfo ().info-> type ()-> is<IVoidInfo> ())
		Ymir::Error::noValueNonVoidFunction (this-> token);
	    else
		Table::instance ().retInfo ().info-> type (new (Z0)  IVoidInfo ());
	}
	
	aux-> verifLocal ();
	return aux;
    }

    std::vector <semantic::Symbol> ITupleDest::allInnerDecls () {
	std::vector <Symbol> syms;
	for (auto it : this-> decls)
	    syms.push_back (it-> info);
	return syms;
    }
    
    Instruction ITupleDest::Incompatible (semantic::Symbol info) {
	auto tuType = new (Z0) ITupleInfo (true);
	for (auto it __attribute__((unused)) : Ymir::r (0, this-> decls.size ()))
		tuType-> addParam (new (Z0) IUndefInfo ());
	Ymir::Error::incompatibleTypes (this-> token, info, tuType);
	return NULL;
    }

    Instruction ITupleDest::instruction () {
	auto right = this-> right-> expression ();
	if (right == NULL) return NULL;
	if (!right-> info-> type ()-> is<ITupleInfo> ()) {	    
	    return this-> Incompatible (right-> info);
	}

	auto tupleType = right-> info-> type ()-> to <ITupleInfo> ();
	Word aff {this-> token.getLocus (), Token::EQUAL};
	std::vector <Expression> insts;

	if (!this-> isVariadic && this-> decls.size () != tupleType-> nbParams ()) {
	    return this-> Incompatible (right-> info);
	} else if (this-> isVariadic && this-> decls.size () > tupleType-> nbParams ()) {
	    return this-> Incompatible (right-> info);
	}

	auto auxDecl = new (Z0) ITupleDest (this-> token, {}, right);		
	ulong i = 0;
	for (auto it : this-> decls) {
	    auto aux = new (Z0) IVar (it-> token);
	    auto info = Table::instance ().get (it-> token.getStr ());
	    if (info && Table::instance ().sameFrame (info)) {
		Ymir::Error::shadowingVar (it-> token, info-> sym);
		return NULL;
	    }

	    aux-> info = new (Z0) ISymbol (aux-> token, aux, new (Z0) IUndefInfo ());
	    aux-> info-> isConst (false);
	    auxDecl-> decls.push_back (aux);
	    
	    Table::instance ().insert (aux-> info);
	    if (i == this-> decls.size () - 1 && this-> isVariadic) {
		std::vector <Expression> last;
		for (auto it_ : Ymir::r (i, tupleType-> nbParams ())) {
		    auto exp = new (Z0) IExpand (right-> token, right, it_);
		    exp-> info = new (Z0) ISymbol (exp-> token, exp, tupleType-> getParams () [it_]-> clone ());
		    last.push_back (exp);
		}

		auto ctuple = new (Z0) IConstTuple (right-> token, right-> token, last);
		auxDecl-> insts.push_back ((new (Z0) IBinary (aff, new (Z0) IVar (this-> decls [i]-> token), ctuple))-> expression ());
	    } else {
		auto exp = new (Z0) IExpand (right-> token, right, i);
		exp-> info = new (Z0) ISymbol (exp-> token, exp, tupleType-> getParams () [i]-> clone ());
		auxDecl-> insts.push_back ((new (Z0) IBinary (aff, new (Z0) IVar (it-> token), exp))-> expression ());
	    }
	    i ++;
	}
	
	return auxDecl;
    }

    Instruction IMatch::instruction () {
	Table::instance ().enterBlock ();
	auto name = Ymir::OutBuffer ("_", ISymbol::getLastTmp ()).str ();

	auto aux = new (Z0) IVar ({{this-> token, name}, name});
	auto expr = this-> _expr-> expression ();

	if (expr == NULL) return NULL;
	aux-> info = new (Z0) ISymbol (aux-> token, aux, new (Z0) IRefInfo (expr-> info-> isConst ()
									    , expr-> info-> type ()-> clone ()));
	Table::instance ().insert (aux-> info);
	
	Word affTok {this-> token, Token::EQUAL};
	auto binAux = (new (Z0) IAffectGeneric (affTok, aux, expr, true))-> expression ();
	if (!binAux) return NULL;       
	aux-> info-> value () = expr-> info-> value ();
	
	std::vector <semantic::DestructSolution> soluce;
	std::vector <Block> blocks;
	bool unreachable = false;
	for (auto it : Ymir::r (0, this-> _values.size ())) {
	    Table::instance ().enterBlock ();
	    if (unreachable) {
		Ymir::Error::unreachableStmtWarn (this-> _values [it]-> token);
	    } 
	    auto res = semantic::DestructSolver::instance ().solve (this-> _values [it], aux);	    
	    if (res.valid) {
		soluce.push_back (res);
		Table::instance ().retInfo ().currentBlock () = "if";
		Table::instance ().enterBlock ();
		for (auto it_ : res.created) {
		    Table::instance ().insert (it_-> info);
		}
		blocks.push_back (this-> _block [it]-> block ());		
		Table::instance ().quitBlock ();
		unreachable = res.immutable;
	    }
	    Table::instance ().quitBlock ();
	}	

	Table::instance ().quitBlock ();
	auto ret = new (Z0) IMatch (this-> token, expr);	
	ret-> _aux = aux;		
	ret-> _binAux = binAux;
	ret-> _soluce = soluce;
	ret-> _block = blocks;
	return ret;
    }

    std::vector <semantic::Symbol> IScope::allInnerDecls () {
	return this-> _block-> allInnerDecls ();
    }
    
    Instruction IScope::instruction () {
	if (this-> token == Keys::EXIT) {
	    Table::instance ().retInfo ().currentBlock () = "if";
	    auto ret = this-> _block-> block ();
	    this-> father ()-> addFinally (ret);	
	    return new (Z0) INone (this-> token);
	} else if (this-> token == Keys::FAILURE) {
	    Table::instance ().retInfo ().currentBlock () = "if";
	    auto ret = this-> _block-> block ();
	    this-> father ()-> addFinalFailure (ret);	    
	    return new (Z0) INone (this-> token);
	} else {
	    Ymir::Error::undefinedScopeEvent (this-> token);
	    return NULL;
	}
    }
    
    std::vector <semantic::Symbol> IScopeFailure::allInnerDecls () {	
	return {};
    }
    
    Instruction IScopeFailure::instruction () {
	for (auto it : Ymir::r (0, this-> _vars.size ())) {
	    auto var = this-> _vars [it]-> templateExpReplace ({})-> to <ITypedVar> ();
	    Table::instance ().retInfo ().currentBlock () = "if";
	    Table::instance ().enterBlock ();

	    
	    auto type = var-> getType ();
	    if (type == NULL) return NULL;
	    auto info = Table::instance ().get (var-> token.getStr ());
	    if (info && Table::instance ().sameFrame (info)) {
		Ymir::Error::shadowingVar (var-> token, info-> sym);
		return NULL;
	    }
	    
	    var-> info = new (Z0) ISymbol (var-> token, var, type);
	    var-> info-> value () = NULL;
	    Table::instance ().insert (var-> info);
	    
	    auto ret = this-> _blocks [it]-> block ();
	    Table::instance ().quitBlock ();
	    this-> father ()-> addFailure (new (Z0) IFailureBlock (this-> token, ret, var));	
	}

	if (this-> _block) {
	    Table::instance ().retInfo ().currentBlock () = "if";
	    auto ret = this-> _block-> block ();
	    this-> father ()-> addFinalFailure (ret);	    
	}
	
	return new (Z0) INone (this-> token);
    }
    
}

