#include <ymir/ast/_.hh>
#include <ymir/semantic/pack/Table.hh>

namespace syntax {
        
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

    void IFunction::print (int nb) {
	printf ("\n%*c<Function> %s ",
		nb, ' ',
		this-> ident.toString ().c_str ()
	);
	if (this-> type) {
	    printf ("\n%*c ->", nb, ' ');
	    this-> type-> print (nb + 4);
	}

	if (this-> test) {
	    printf ("\n%*c if (", nb + 2, ' ');
	    this-> test-> print (nb + 4);
	    printf ("\n%*c )", nb + 2, ' ');
	}
	    
	printf ("\n%*c !( ", nb + 2, ' ');
	for (auto it : this-> tmps) {
	    it-> print (nb + 4);
	}
	printf ("\n%*c )", nb + 2, ' ');

	printf ("\n%*c (", nb + 2, ' ');
	for (auto it : this-> params) {
	    it-> print (nb + 4);
	}
	printf ("\n%*c )", nb + 2, ' ');

	this-> block-> print (nb + 6);	    	    
    }
    
    void IImpl::print (int nb) {
	printf ("\n%*c<Impl> %s : %s",
		nb, ' ',
		this-> what.toString ().c_str (),
		this-> who.isEof () ? "object" : this-> who.toString ().c_str ()
	);

	for (auto it : this-> csts) {
	    it-> print (nb + 4);
	}
	    
	for (auto it : this-> methods) {
	    it-> print (nb + 4);
	}	    
    }

    
	
    void IParamList::print (int nb) {
	printf ("\n%*c<ParamList> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);

	for (auto it : this-> params) {
	    it-> print (nb + 4);
	}	    
    }


    void IVar::print (int nb) {
	printf ("\n%*c<Var> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	    
	for(auto it : this-> templates) {
	    it-> print (nb + 4);
	}
	    
    }	
    
    void IType::print (int nb) {
	printf ("\n%*c<Type> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
    }
    
    void ITypedVar::print (int nb) {
	printf ("\n%*c<TypedVar> %s%s",
		nb, ' ',
		this-> deco.isEof () ? "" : this-> deco.getStr ().c_str (),
		this-> token.toString ().c_str ()
	);
	if (this-> type) 
	    this-> type-> print (nb + 4);
	else this-> expType-> print (nb + 4);
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
