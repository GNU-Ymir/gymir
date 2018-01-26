#include <ymir/ast/_.hh>
#include <ymir/semantic/pack/_.hh>
#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/value/_.hh>
#include <ymir/syntax/Keys.hh>

namespace syntax {

    using namespace semantic;
    
    Instruction IBlock::instruction () {
	return this-> block ();
    }

    Block IBlock::block () {
	Table::instance ().enterBlock ();
	auto ret = blockWithoutEnter ();
	Table::instance ().quitBlock ();
	return ret;
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
		Ymir::Error::unreachableStmt (it-> token);
		break;
	    }
	    if (!it-> is<INone> ()) {
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

    
    Instruction IVarDecl::instruction () {
	auto auxDecl = new (Z0)  IVarDecl (this-> token);
	for (auto id : Ymir::r (0, this-> decls.size ())) {
	    auto it = this-> decls [id];
	    auto aux = new (Z0)  IVar (it-> token);
	    auto info = Table::instance ().get (it-> token.getStr ());
	    if (info && Table::instance ().sameFrame (info)) {
		Ymir::Error::shadowingVar (it-> token, info-> sym);
		return NULL;
	    }

	    if (this-> decos [id] == Keys::IMMUTABLE) {
		Ymir::Error::assert ("TODO, Immut");
	    } else if (this-> decos [id] == Keys::CONST) {
		aux-> info = new (Z0)  ISymbol (aux-> token, new (Z0)  IUndefInfo ());
		aux-> info-> isConst (false);
		Table::instance ().insert (aux-> info);
		if (this-> insts [id] == NULL) {
		    Ymir::Error::constNoInit (it-> token);
		    return NULL;
		} else {
		    auxDecl-> decls.push_back (aux);
		    auxDecl-> insts.push_back (this-> insts [id]-> expression ());
		    aux-> info-> isConst (true);
		}
	    } else if (this-> decos [id] == Keys::STATIC) {
		Ymir::Error::assert ("TODO, static");
	    } else {
		aux-> info = new (Z0)  ISymbol (aux-> token, new (Z0)  IUndefInfo ());
		aux-> info-> isConst (false);
		Table::instance ().insert (aux-> info);
		auxDecl-> decls.push_back (aux);
		if (this-> insts [id]) 
		    auxDecl-> insts.push_back (this-> insts [id]-> expression ());
		else auxDecl-> insts.push_back (NULL);		
	    }
	}
	
	return auxDecl;
    }

    Instruction IAssert::instruction () {
	auto expr = this-> expr-> expression ();
	if (!expr-> info-> type-> isSame (new (Z0)  IBoolInfo (true))) {
	    Ymir::Error::incompatibleTypes (this-> token, expr-> info, new (Z0)  IBoolInfo (true));
	    return NULL;
	}

	Expression msg = NULL;
	if (this-> msg) {
	    msg = this-> msg-> expression ();
	    if (!msg-> info-> type-> isSame (new (Z0)  IStringInfo (true))) {
		Ymir::Error::incompatibleTypes (this-> token, expr-> info, new (Z0)  IStringInfo (true));
		return NULL;
	    }
	}

	if (this-> isStatic) {
	    if (msg && !msg-> info-> isImmutable ()) {
		Ymir::Error::assert ("TODO");
	    }

	    if (!expr-> info-> isImmutable ()) {
		Ymir::Error::assert ("TODO");
	    } else if (!(expr-> info-> value ()-> to<IBoolValue> ()-> isTrue ())) {
		Ymir::Error::assert ("TODO");
	    }
	} else if (expr-> info-> isImmutable ()) {
	    if (expr-> info-> value ()-> to<IBoolValue> ()-> isTrue ())
		Table::instance ().retInfo ().returned ();
	}
	return new (Z0)  IAssert (this-> token, expr, msg, this-> isStatic);
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
		    Ymir::Error::notImmutable (expr-> info);
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
    
    Instruction IFor::instruction () {
	Ymir::Error::assert ("TODO For");
	return NULL;
    }
    
    Instruction IWhile::instruction () {
	auto expr = this-> test-> expression ();
	if (expr == NULL) return NULL;
	auto type = expr-> info-> type-> CompOp (new (Z0)  IBoolInfo (true));

	if (type == NULL) {
	    Ymir::Error::incompatibleTypes (this-> token, expr-> info, new (Z0)  IBoolInfo (true));
	}

	Table::instance ().retInfo ().currentBlock () = "while";
	Table::instance ().retInfo ().changed () = true;
	Block bl = this-> block-> block ();

	return new (Z0)  IWhile (this-> token, expr, bl);		
    }


    Instruction IReturn::instruction () {
	auto aux = new (Z0)  IReturn (this-> token);
	Table::instance ().retInfo ().returned ();
	if (this-> elem != NULL) {
	    aux-> elem = this-> elem-> expression ();
	    if (aux-> elem == NULL) return NULL;
	    if (aux-> elem-> info-> type-> is <IVoidInfo> ()) {
		Ymir::Error::returnVoid (this-> token, aux-> elem-> info);
	    }
	    
	    if (Table::instance ().retInfo ().info-> type-> is <IUndefInfo> ()) {
		Table::instance ().retInfo ().info-> type = aux-> elem-> info-> type-> clone ();
		if (!Table::instance ().retInfo ().changed ()) {
		    Table::instance ().retInfo ().changed () = true;
		    Table::instance ().retInfo ().info-> value () = aux-> elem-> info-> value ();
		} else 
		    Table::instance ().retInfo ().info-> value () = NULL;
	    } else {
		auto type = aux-> elem-> info-> type-> CompOp (Table::instance ().retInfo ().info-> type);
		if (type == NULL) {
		    Ymir::Error::incompatibleTypes (this-> token, aux-> elem-> info, Table::instance ().retInfo ().info-> type);
		} else if (type-> isSame (aux-> elem-> info-> type)) {		    
		    if (!Table::instance ().retInfo ().changed ()) {
			Table::instance ().retInfo ().info-> value () =
			    aux-> elem-> info-> value ();
		    } else
			Table::instance ().retInfo ().info-> value () = NULL;
		    Table::instance ().retInfo ().changed () = true;
		}
	    
	    }
	    
	    aux-> caster = aux-> elem-> info-> type-> CompOp (Table::instance ().retInfo ().info-> type);
	    if (!Table::instance ().retInfo ().info-> type-> is <IRefInfo> ())
		aux-> caster-> isConst (true);			    
	} else {
	    if (!Table::instance ().retInfo ().info-> type-> is<IUndefInfo> () &&
		!Table::instance ().retInfo ().info-> type-> is<IVoidInfo> ())
		Ymir::Error::noValueNonVoidFunction (this-> token);
	    else
		Table::instance ().retInfo ().info-> type = new (Z0)  IVoidInfo ();
	}
	return aux;
    }


}

