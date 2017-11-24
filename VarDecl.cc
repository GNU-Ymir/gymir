#include "ast/VarDecl.hh"
#include "ast/Var.hh"

namespace syntax {

    Instruction Instruction::instruction () {
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
	    }
	}
	
    }
    
    void IVarDecl::print (int nb) {
	printf ("\n%*c<VarDecl> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);

	for (int it = 0 ; it < this-> decls.size () ; it ++) {
	    
	    this-> decls [it]-> print (nb + 4);
	    if (this-> insts [it]) {
		this-> insts [it]-> print (nb + 6);
	    }
	}	    
    }    
    
}
