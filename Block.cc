#include "ast/Block.hh"
#include "ast/Declaration.hh"
#include <ymir/semantic/pack/Table.hh>

namespace syntax {

    using namespace semantic;

    Word& IBlock::getIdent () {
	return this-> ident;
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
    
    void IBlock::print (int nb) {
	printf ("\n%*c<Block> : %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	
	for (auto it : this-> decls)
	    it-> print (nb + 4);
	
	for (auto it : this-> insts)
	    it-> print (nb + 4);	
    }
    
}
