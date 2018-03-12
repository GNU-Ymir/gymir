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
	if (this-> insts.size () != 0) {
	    return this-> insts.back ()-> to <IExpression> ();
	}
	return NULL;
    }

    std::vector <Symbol> IBlock::allInnerDecls () {
	std::vector <Symbol> allInner;
	for (auto it : this-> insts) {
	    auto decls = it-> allInnerDecls ();
	    allInner.insert (allInner.end (), decls.begin (), decls.end ());
	}
	return allInner;
    }
    
    Block IBlock::blockWithoutEnter () {
	std::vector <Instruction> insts;
	auto bl = new (Z0)  IBlock (this-> token, {}, {});
	
	if (!this-> ident.isEof ()) 
	    Table::instance ().retInfo ().setIdent (this-> ident);

	for (auto it : this-> decls) {
	    it-> declareAsInternal ();
	}

	for (auto it : this-> insts) {
	    if (Table::instance ().retInfo ().hasReturned () ||
		Table::instance ().retInfo ().hasBreaked ()) {
		if (it-> is<IScope> ()) {
		    Ymir::Error::scopeExitEnd (it-> token);
		} else {
		    Ymir::Error::unreachableStmt (it-> token);
		    break;
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
	
	bl-> insts = insts;
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

	    if (this-> decos [id] == Keys::IMMUTABLE) {
		if (auto bin = this-> insts [id]-> to<IBinary> ()) {
		    auto type = bin-> getRight ()-> expression ();
		    if (type == NULL) return NULL;
		    aux-> info = new (Z0) ISymbol (aux-> token, type-> info-> type-> clone ());
		    aux-> info-> isConst (true);
		    aux-> info-> value () = type-> info-> type-> value ();
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
	    } else if (this-> decos [id] == Keys::CONST) {
		aux-> info = new (Z0)  ISymbol (aux-> token, new (Z0)  IUndefInfo ());
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
	    } else if (this-> decos [id] == Keys::STATIC) {
		auto space = Table::instance ().space ();
		if (auto bin = this-> insts [id]-> to<IBinary> ()) {
		    auto type = bin-> getRight ()-> expression ();
		    aux-> info = new (Z0) ISymbol (aux-> token, type-> info-> type-> clone ());
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
		aux-> info = new (Z0)  ISymbol (aux-> token, new (Z0)  IUndefInfo ());
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
	auto expr = this-> expr-> expression ();
	if (expr == NULL) return NULL;
	if (!expr-> info-> type-> isSame (new (Z0)  IBoolInfo (true))) {
	    Ymir::Error::incompatibleTypes (this-> token, expr-> info, new (Z0)  IBoolInfo (true));
	    return NULL;
	}
	
	Expression msg = NULL;
	if (this-> msg) {
	    msg = this-> msg-> expression ();
	    if (!msg-> info-> type-> isSame (new (Z0) IStringInfo (true))) {
		Ymir::Error::incompatibleTypes (this-> token, expr-> info, new (Z0) IStringInfo (true));
		return NULL;
	    }
	}

	if (this-> isStatic) {
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
	return new (Z0)  IAssert (this-> token, expr, msg, this-> isStatic);
    }
    
    std::vector <semantic::Symbol> IAssert::allInnerDecls () {
	return {};
    }

    std::vector <semantic::Symbol> IBreak::allInnerDecls () {
	return {};
    }

    Instruction IBreak::instruction () {
	auto aux = new (Z0)  IBreak (this-> token, this-> ident);
	Table::instance ().retInfo ().breaked ();
	if (this-> ident.isEof ()) {
	    auto nb = Table::instance ().retInfo ().rewind (std::vector <std::string> {"while", "for"});
	    if (nb == -1) {
		Ymir::Error::breakOutSide (this-> token);
		return NULL;
	    } else {
		aux-> nbBlock = nb;
	    }
	} else {
	    auto nb = Table::instance ().retInfo ().rewind (this-> ident.getStr ());
	    if (nb == -1) {
		Ymir::Error::breakRefUndef (this-> ident);
		return NULL;
	    } else aux-> nbBlock = nb;
	}
	return aux;
    }    

    std::vector <semantic::Symbol> IIf::allInnerDecls () {
	auto ret = this-> block-> allInnerDecls ();
	auto el = this-> else_-> allInnerDecls ();
	ret.insert (ret.begin (), el.begin (), el.end ());
	return ret;
    }
    
    Instruction IIf::instruction () {
	if (this-> test != NULL) {
	    auto expr = this-> test-> expression ();
	    if (expr == NULL) return NULL;
	    auto type = expr-> info-> type-> CompOp (new (Z0)  IBoolInfo (true));
	    
	    if (type == NULL) {
		Ymir::Error::incompatibleTypes (this-> token, expr-> info, new (Z0)  IBoolInfo (true));
		return NULL;
	    }


	    if (this-> isStatic) {
		if (!expr-> info-> isImmutable ()) {
		    Ymir::Error::notImmutable (this-> token, expr-> info);
		    return NULL;
		} else if (expr-> info-> value ()-> to<IBoolValue> ()-> isTrue ()) {
		    return this-> block-> instruction ();
		} else {
		    if (this-> else_) {
			this-> else_-> isStatic  = this-> isStatic;
			return this-> else_-> instruction ();
		    }
		    else return new (Z0)  IBlock (this-> block-> token, {}, {});
		}
	    }
	    
	    Table::instance ().retInfo ().currentBlock () = "if";
	    Table::instance ().retInfo ().changed () = true;
	    Block bl = this-> block-> block ();
	    If _if;
	    if (this-> else_ != NULL) {
		_if = new (Z0)  IIf (this-> token, expr, bl, (If) this-> else_-> instruction ());
		_if-> info = type;
	    } else {
		_if = new (Z0)  IIf (this-> token, expr, bl);
		_if-> info = type;
	    }
	    return _if;
	} else {
	    if (!this-> isStatic) {
		Table::instance ().retInfo ().currentBlock () = "else";
		Table::instance ().retInfo ().changed () = true;
	    
		Block bl = this-> block-> block ();
		If _if = new (Z0)  IIf (this-> token, NULL, bl);
		return _if;
	    } else {
		return this-> block-> instruction ();
	    }
	}
    }

    std::vector <semantic::Symbol> IFor::allInnerDecls () {
	std::vector <Symbol> syms;
	for (auto it : this-> var)
	    syms.push_back (it-> info);
	auto bl = this-> block-> allInnerDecls ();
	syms.insert (syms.end (), bl.begin (), bl.end ());
	return syms;
    }
    
    Instruction IFor::immutable (Expression expr) {
	if (expr-> info-> type-> is<IRangeInfo> () && this-> var.size () == 1) {
	    return immutableRange (this-> var, expr);
	} else if (expr-> info-> type-> is <ITupleInfo> () && this-> var.size () == 1) {
	    return immutableTuple (this-> var, expr);
	}
	
	Ymir::Error::undefinedOp (this-> token, expr-> info);
	return NULL;
    }

    Instruction IFor::immutableTuple (std::vector <Var> & vars, Expression expr) {
	auto tu = expr-> info-> type-> to <ITupleInfo> ();
	Table::instance ().enterBlock ();
	Block bl = new (Z0) IBlock (this-> token, {}, {});
	auto varDecl = new IVarDecl (this-> token, {}, {}, {});
	bl-> getInsts ().push_back (varDecl);
	
	auto aux = new (Z0) IVar ({this-> token, "_"});
	aux-> info = new (Z0) ISymbol (aux-> token, new (Z0) IUndefInfo ());
	Table::instance ().insert (aux-> info);
	varDecl-> getDecls ().push_back (aux);
	varDecl-> getInsts ().push_back ((new (Z0) IBinary ({this-> token, Token::EQUAL},
						       aux, expr
						       ))-> expression ()
				     );
	
	for (auto i : Ymir::r (0, tu-> nbParams ())) {
	    auto var = vars [0]-> templateExpReplace ({})-> to <IVar> ();
	    Table::instance ().enterBlock ();
	    auto index = new (Z0) IFixedInfo (true, FixedConst::UINT);
	    index-> value () = new (Z0) IFixedValue (FixedConst::UINT, i, i);
	    auto indexVal = index-> value ()-> toYmir (new (Z0) ISymbol (var-> token, index));
	    var-> info = new (Z0) ISymbol (var-> token, new (Z0) IUndefInfo ());
	    var-> info-> isConst (false);
	    Table::instance ().insert (var-> info);
	    
	    varDecl-> getDecls ().push_back (var);
	    varDecl-> getInsts ().push_back (NULL);
	    
	    bl-> getInsts ().push_back ((new (Z0) IBinary ({this-> token, Token::EQUAL},
							  var,
							  new (Z0) IDot (this-> token, aux, indexVal)
							  ))-> expression ());
	    
	    bl-> getInsts ().push_back (this-> block-> block ());
	    Table::instance ().quitBlock ();
	}
	Table::instance ().quitBlock ();
	return bl;
    }
    
    Instruction IFor::immutableRange (std::vector <Var> & vars, Expression expr) {
	auto range = expr-> info-> type-> to <IRangeInfo> ();
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
		    var-> info = new (Z0) ISymbol (var-> token, index);
		    Table::instance ().enterBlock ();
		    Table::instance ().insert (var-> info);
		    bl-> getInsts ().push_back (this-> block-> block ());
		    Table::instance ().quitBlock ();
		}
	    } else {
		long left = li-> getValue (), right = ri-> getValue ();
		for (long i = left ; i != right ; i += left > right ? -1 : 1) {
		    auto var = vars [0]-> templateExpReplace ({});
		    auto index = range-> content ()-> cloneConst ();
		    index-> value () = new (Z0) IFixedValue (li-> getType (), i, i);
		    var-> info = new (Z0) ISymbol (var-> token, index);
		    Table::instance ().enterBlock ();
		    Table::instance ().insert (var-> info);
		    bl-> getInsts ().push_back (this-> block-> block ());
		    Table::instance ().quitBlock ();
		}
	    }
	} else {
	    Ymir::Error::notImmutable (this-> token, expr-> info);
	    return NULL;
	}
	
	return bl;
    }
    
    Instruction IFor::instruction () {
	auto expr = this-> iter-> expression ();
	if (expr == NULL) return NULL;
	if (!this-> id.isEof () && !this-> isStatic)
	    Table::instance ().retInfo ().setIdent (this-> id);
	else if (!this-> id.isEof () && this-> isStatic) {
	    Ymir::Error::labelingImmutableFor (this-> id);
	}
	
	if (!this-> isStatic) {
	    Table::instance ().enterBlock ();
	    std::vector <Var> var (this-> var.size ());
	    for (auto it : Ymir::r (0, this-> var.size ())) {
		var [it] = (Var) this-> var [it]-> templateExpReplace ({});
		auto info = Table::instance ().get (var [it]-> token.getStr ());
		if (info && Table::instance ().sameFrame (info)) {
		    Ymir::Error::shadowingVar (var [it]-> token, info-> sym);
		    return NULL;
		}

		var [it]-> info = new (Z0) ISymbol (var [it]-> token, new (Z0) IUndefInfo ());
		var [it]-> info-> isConst (false);
		var [it]-> info-> value () = NULL;
		if (!this-> isStatic)
		    Table::instance ().insert (var [it]-> info);
	    }
	
	    auto type = expr-> info-> type-> ApplyOp (var);
	    if (type == NULL) {
		Ymir::Error::undefinedOp (this-> token, expr-> info);
		return NULL;
	    }

	    Table::instance ().retInfo ().currentBlock () = "for";
	    Table::instance ().retInfo ().changed () = true;
	    Block bl = this-> block-> block ();
	    Table::instance ().quitBlock ();
	    auto aux = new (Z0) IFor (this-> token, this-> id, var, expr, bl);
	    aux-> ret = type;
	    return aux;
	} else {
	    return this-> immutable (expr);	    
	}
    }

    std::vector <semantic::Symbol> IWhile::allInnerDecls () {
	return this-> block-> allInnerDecls ();
    }
    
    Instruction IWhile::instruction () {
	auto expr = this-> test-> expression ();
	if (expr == NULL) return NULL;
	auto type = expr-> info-> type-> CompOp (new (Z0)  IBoolInfo (true));

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
    
    Instruction IReturn::instruction () {
	auto aux = new (Z0)  IReturn (this-> token);
	Table::instance ().retInfo ().returned ();
	if (this-> elem != NULL) {
	    this-> elem-> inside = this;
	    aux-> elem = this-> elem-> expression ();
	    if (aux-> elem == NULL) return NULL;
	    if (aux-> elem-> info-> type-> is <IVoidInfo> ()) {
		Ymir::Error::returnVoid (this-> token, aux-> elem-> info);
		return NULL;
	    }
	    
	    auto type = aux-> elem-> info-> type-> CompOp (Table::instance ().retInfo ().info-> type);	    
	    if (type == NULL) {		
		Ymir::Error::incompatibleTypes (this-> token, aux-> elem-> info, Table::instance ().retInfo ().info-> type);
		return NULL;
	    }
	    
	    if (Table::instance ().retInfo ().info-> type-> is <IUndefInfo> ())
		Table::instance ().retInfo ().info-> type = type;
	    
	    if (type-> isSame (aux-> elem-> info-> type)) {		
		if (!Table::instance ().retInfo ().changed ()) {
		    Table::instance ().retInfo ().info-> value () =
			aux-> elem-> info-> value ();
		} else
		    Table::instance ().retInfo ().info-> value () = NULL;
		Table::instance ().retInfo ().changed () = true;
	    }
	    	    	    
	    aux-> caster = type-> cloneOnExitWithInfo ();
	    if (Table::instance ().retInfo ().deco == "" || Table::instance ().retInfo ().deco == Keys::CONST)
	    	aux-> caster-> isConst (true);
	    else {
		if (!aux-> caster-> ConstVerif (Table::instance ().retInfo ().info-> type)) {
		    Ymir::Error::incompatibleTypes (this-> token, aux-> elem-> info, Table::instance ().retInfo ().info-> type);
		    return NULL;		    
		}
	    }
	} else {
	    if (!Table::instance ().retInfo ().info-> type-> is<IUndefInfo> () &&
		!Table::instance ().retInfo ().info-> type-> is<IVoidInfo> ())
		Ymir::Error::noValueNonVoidFunction (this-> token);
	    else
		Table::instance ().retInfo ().info-> type = new (Z0)  IVoidInfo ();
	}

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
	if (!right-> info-> type-> is<ITupleInfo> ()) {	    
	    return this-> Incompatible (right-> info);
	}

	auto tupleType = right-> info-> type-> to <ITupleInfo> ();
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

	    aux-> info = new (Z0) ISymbol (aux-> token, new (Z0) IUndefInfo ());
	    aux-> info-> isConst (false);
	    auxDecl-> decls.push_back (aux);
	    
	    Table::instance ().insert (aux-> info);
	    if (i == this-> decls.size () - 1 && this-> isVariadic) {
		std::vector <Expression> last;
		for (auto it_ : Ymir::r (i, tupleType-> nbParams ())) {
		    auto exp = new (Z0) IExpand (right-> token, right, it_);
		    exp-> info = new (Z0) ISymbol (exp-> token, tupleType-> getParams () [it_]-> clone ());
		    last.push_back (exp);
		}

		auto ctuple = new (Z0) IConstTuple (right-> token, right-> token, last);
		auxDecl-> insts.push_back ((new (Z0) IBinary (aff, new (Z0) IVar (this-> decls [i]-> token), ctuple))-> expression ());
	    } else {
		auto exp = new (Z0) IExpand (right-> token, right, i);
		exp-> info = new (Z0) ISymbol (exp-> token, tupleType-> getParams () [i]-> clone ());
		auxDecl-> insts.push_back ((new (Z0) IBinary (aff, new (Z0) IVar (it-> token), exp))-> expression ());
	    }
	    i ++;
	}
	
	return auxDecl;
    }

    Instruction IMatch::instruction () {
	Table::instance ().enterBlock ();
	auto aux = new (Z0) IVar ({expr-> token, "_"});
	aux-> info = new (Z0) ISymbol (aux-> token, new (Z0) IUndefInfo ());
	aux-> info-> isConst (false);
	Table::instance ().insert (aux-> info);
	
	Word affTok {this-> token, Token::EQUAL};
	auto binAux = (new (Z0) IBinary (affTok, aux, this-> expr))-> expression ();
	if (!binAux) return NULL;       
	aux-> info-> isConst (true);
	aux-> info-> value () = binAux-> to<IBinary> ()-> getRight ()-> info-> value ();
	std::vector <semantic::DestructSolution> soluce;
	std::vector <Block> blocks;
	bool unreachable = false;
	for (auto it : Ymir::r (0, this-> values.size ())) {
	    Table::instance ().enterBlock ();
	    if (unreachable) {
		Ymir::Error::unreachableStmtWarn (this-> values [it]-> token);
	    } 
	    auto res = semantic::DestructSolver::instance ().solve (this-> values [it], aux);	    
	    if (res.valid) {
		soluce.push_back (res);
		Table::instance ().enterBlock ();
		for (auto it_ : res.created) {
		    Table::instance ().insert (it_-> info);
		}
		blocks.push_back (this-> block [it]-> block ());		
		Table::instance ().quitBlock ();
		unreachable = res.immutable;
	    }
	    Table::instance ().quitBlock ();
	}	

	Table::instance ().quitBlock ();
	auto ret = new (Z0) IMatch (this-> token, expr);	
	ret-> aux = aux;		
	ret-> binAux = binAux;
	ret-> soluce = soluce;
	ret-> block = blocks;
	return ret;
    }

    std::vector <semantic::Symbol> IScope::allInnerDecls () {
	return this-> block-> allInnerDecls ();
    }
    
    Instruction IScope::instruction () {
	Table::instance ().retInfo ().currentBlock () = "if";
	auto ret = block-> block ();
	this-> father ()-> addFinally (ret);	
	return new (Z0) INone (this-> token);
    }
    

}

