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
	auto bl = new IBlock (this-> token, {}, {});
	
	if (!this-> ident.isEof ()) 
	    Table::instance ().retInfo ().setIdent (this-> ident);

	for (auto it : this-> decls) {
	    it-> declareAsInternal ();
	}

	for (auto it : this-> insts) {
	    if (Table::instance ().retInfo ().hasReturned () ||
		Table::instance ().retInfo ().hasBreaked ()) {
		Ymir::Error::assert ("TODO");
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
	auto auxDecl = new IVarDecl (this-> token);
	ulong id = 0;
	for (auto it : this-> decls) {
	    auto aux = new IVar (it-> token);
	    auto info = Table::instance ().get (it-> token.getStr ());
	    if (info && Table::instance ().sameFrame (info)) {
		Ymir::Error::shadowingVar (it-> token, info-> sym);
		return NULL;
	    }

	    if (this-> decos [id] == Keys::IMMUTABLE) {
		Ymir::Error::assert ("TODO, Immut");
	    } else if (this-> decos [id] == Keys::CONST) {
		aux-> info = new ISymbol (aux-> token, new IUndefInfo ());
		Table::instance ().insert (aux-> info);
	    } else if (this-> decos [id] == Keys::STATIC) {
		Ymir::Error::assert ("TODO, static");
	    } else {
		aux-> info = new ISymbol (aux-> token, new IUndefInfo ());
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
	if (!expr-> info-> type-> isSame (new IBoolInfo (true))) {
	    Ymir::Error::incompatibleTypes (this-> token, expr-> info, new IBoolInfo (true));
	    return NULL;
	}

	Expression msg;
	if (this-> msg) {
	    msg = this-> msg-> expression ();
	    if (!msg-> info-> type-> isSame (new IStringInfo (true))) {
		Ymir::Error::incompatibleTypes (this-> token, expr-> info, new IStringInfo (true));
		return NULL;
	    }
	}

	if (this-> isStatic) {
	    if (msg && !msg-> info-> isImmutable ()) {
		Ymir::Error::assert ("TODO");
	    }

	    if (!expr-> info-> isImmutable ()) {
		Ymir::Error::assert ("TODO");
	    } else if (!(expr-> info-> value-> to<IBoolValue> ()-> isTrue ())) {
		Ymir::Error::assert ("TODO");
	    }
	} else if (expr-> info-> isImmutable ()) {
	    if (expr-> info-> value-> to<IBoolValue> ()-> isTrue ())
		Table::instance ().retInfo ().returned ();
	}
	return new IAssert (this-> token, expr, msg, this-> isStatic);
    }
    
    Instruction IBreak::instruction () {
	auto aux = new IBreak (this-> token, this-> ident);
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

}

