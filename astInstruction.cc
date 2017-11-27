#include <ymir/ast/_.hh>
#include <ymir/semantic/pack/_.hh>
#include <ymir/semantic/types/_.hh>
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
		insts.push_back (it-> instruction ());
		insts.back ()-> father () = bl;
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
	    if (info && Table::instance ().sameFrame (info))
		Ymir::Error::assert ("TODO, Shadowing");


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

    
}

