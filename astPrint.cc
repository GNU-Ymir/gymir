#include <ymir/ast/_.hh>
#include <ymir/semantic/pack/Table.hh>

namespace syntax {
        
    void IBlock::print (int nb) {
	printf ("\n%*c<Block> : %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	
	for (auto it : this-> _decls)
	    it-> print (nb + 4);
	
	for (auto it : this-> _insts)
	    it-> print (nb + 4);	
    }

    void IFunction::print (int nb) {
	printf ("\n%*c<Function> %s ",
		nb, ' ',
		this-> _ident.toString ().c_str ()
	);
	if (this-> _type) {
	    printf ("\n%*c ->", nb, ' ');
	    this-> _type-> print (nb + 4);
	}

	if (this-> _test) {
	    printf ("\n%*c if (", nb + 2, ' ');
	    this-> _test-> print (nb + 4);
	    printf ("\n%*c )", nb + 2, ' ');
	}
	    
	printf ("\n%*c !( ", nb + 2, ' ');
	for (auto it : this-> _tmps) {
	    it-> print (nb + 4);
	}
	printf ("\n%*c )", nb + 2, ' ');

	printf ("\n%*c (", nb + 2, ' ');
	for (auto it : this-> _params) {
	    it-> print (nb + 4);
	}
	printf ("\n%*c )", nb + 2, ' ');

	this-> _block-> print (nb + 6);	    	    
    }
       
	
    void IParamList::print (int nb) {
	printf ("\n%*c<ParamList> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);

	for (auto it : this-> _params) {
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
	if (this-> info)
	    printf (" %s", this-> info-> typeString ().c_str ());
	
    }	
    
    void IType::print (int nb) {
	printf ("\n%*c<Type> %s : %s",
		nb, ' ',
		this-> token.toString ().c_str (),
		this-> _type-> typeString ().c_str ()
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
    }

   
    
    void IVarDecl::print (int nb) {
	printf ("\n%*c<VarDecl> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);

	for (uint it = 0 ; it < this-> decls.size () ; it ++) {
	    
	    this-> decls [it]-> print (nb + 4);
	    if (this-> insts [it]) {
		this-> insts [it]-> print (nb + 6);
	    }
	}	    
    }    

    void IArrayAlloc::print (int nb) {
	printf ("\n%*c<ArrayAlloc> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	    
	if (this-> _type)
	    this-> _type-> print (nb + 4);
	this-> _size-> print (nb + 4);
    }	

    

    void IAssert::print (int nb) {
	printf ("\n%*c<Assert> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	    
	this-> _expr-> print (nb + 4);
	if (this-> _msg) this-> _msg-> print (nb + 4);
    }	

    void IBinary::print (int nb) {
	printf ("\n%*c<Binary> : %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);	

	if (this-> info) printf (" -> %s", this-> info-> typeString ().c_str ());	
	this-> _left-> print (nb + 4);
	this-> _right-> print (nb + 4);
    }	

    void IFixed::print (int nb) {
	printf ("\n%*c<Fixed> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);	    
    }	
    
    void IChar::print (int nb) {
	printf ("\n%*c<Char> %s %d:(%c)",
		nb, ' ',
		this-> token.toString ().c_str (),
		this-> _code, (char) (this-> _code)
	);
    }


    void IFloat::print (int nb) {
	printf ("\n%*c<Float> %s [%s]",
		nb, ' ', this-> token.toString ().c_str (),
		this-> _totale.c_str ()
	);
    }
    
    void IString::print (int nb) {
	printf ("\n%*c<String> %s : [%s]",
		nb, ' ',
		this-> token.toString ().c_str (),
		this-> _content.c_str ()
	);	    
    }    
    	
    void IBreak::print (int nb) {
	printf ("\n%*c<Break> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);		  	    
    }

    void ICast::print (int nb) {
	printf ("\n%*c<Cast> %s", nb, ' ', this-> token.toString ().c_str ());
	this-> _type-> print (nb + 4);
	this-> _expr-> print (nb + 4);	
    }

    void IConstArray::print (int nb) {
	printf ("\n%*c<ConstArray> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
    }	
    
   
    void IDColon::print (int nb) {
	printf ("\n%*c<DColon> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	this-> _left-> print (nb + 4);
	this-> _right-> print (nb + 4);
    }

    
    void IDot::print (int nb) {
	printf ("\n%*c<Dot> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	this-> _left-> print (nb + 4);
	this-> _right-> print (nb + 4);
    }

    void IDotCall::print (int nb) {
	printf ("\n%*c<DotCall> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	
	this-> _firstPar-> print (nb + 4);
	this-> _call-> print (nb + 4);
    }

    void IPar::print (int nb) {
	printf ("\n%*c<Par> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	
	this-> _left-> print (nb + 4);
	this-> _params-> print (nb + 4);
    }

    void IProto::print (int nb) {
	printf ("\n%*c<Proto> %s%s",
		nb, ' ',
		this-> _space.c_str (),
		this-> _ident.toString ().c_str ()
	);
	
	for (auto it : this-> _params) {
	    it-> print (nb + 4);
	}
	
	if (this-> _type)
	    this-> _type-> print (nb + 6);
    }
    
    void ILambdaFunc::print (int nb)  {
	printf ("\n%*c<LambdaFunc> %s (",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	    
	for (auto it : this-> _params) {
	    it-> print (nb + 4);
	}
	printf ("\n%*c ) %s ",
		nb, ' ',
		this-> _ret ? "->" : "=>"
	);
	    
	if (this-> _ret) {
	    this-> _ret-> print (nb + 4);
	    this-> _block-> print (nb + 8);
	} else if (this-> _block) {
	    this-> _block-> print (nb + 4);
	} else {
	    this-> _expr-> print (nb + 8);
	}	    		    
    }

    void IFuncPtr::print (int nb) {
	printf ("\n%*c<FuncPtr> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	    
	for (auto it : this-> _params) {
	    it-> print (nb + 4);
	}
	
	this-> _ret-> print (nb + 5);
    }

    void IStruct::print (int nb)  {
	printf ("\n%*c<Struct> %s",
		nb, ' ',
		this-> ident.toString ().c_str ()
	);
	
	printf ("\n%*c!(", nb, ' ');
	for (auto it : this-> tmps)
	    it-> print (nb + 4);
	printf ("\n%*c)", nb, ' ');

	printf ("\n%*c{", nb, ' ');
	for (auto it : this-> params)
	    it-> print (nb + 4);
	printf ("\n%*c}", nb, ' ');	    
    }
        
    void IIs::print (int nb) {
	printf ("\n%*c<Is> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	
	this-> _left-> print (nb + 4);
	if (this-> _type) this-> _type-> print (nb + 4);
	else
	    printf (": %s", this-> _expType.toString ().c_str ());	    
    }
	
    void ITupleDest::print (int nb) {
	printf ("\n%*c<TupleDest> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);

	for (auto it : this-> decls) {
	    it-> print (nb + 4);		
	}

	for (auto it : this-> insts) {
	    it-> print (nb + 4);
	}

	this-> right-> print (nb + 4);	    
    }
        
    void IEnum::print (int nb) {
	printf ("\n%*c<Enum> %s",
		nb, ' ',
		this-> ident.toString ().c_str ()
	);
	
	for (int i = 0 ; i < (int) this-> _names.size (); i++) {
	    printf ("\n%*c%s ", nb + 4, ' ',
		    this-> _names [i].toString ().c_str ());
	    
	    this-> _values [i]-> print (nb + 8);
	}
    }

    void ITypeOf::print (int nb) {
	printf ("\n%*c<TypeOf> %s",
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	this-> expr-> print (nb + 4);
    }
	            	
    void IMatch::print (int nb) {
	printf ("\n%*c<Match> %s", 
		nb, ' ',
		this-> token.toString ().c_str ()
	);
	this-> _expr-> print (nb + 4);
	    
	for (int i = 0 ; i < (int) this-> _values.size () ; i++) {
	    this-> _values [i]-> print (nb + 8);
	    this-> _block [i]-> print (nb + 8);
	}
    }	

    void IGlobal::print (int nb) {
	printf ("\n%*c<Global> %s",
		nb, ' ',
		this-> _ident.toString ().c_str ()
	);

	if (this-> _expr)
	    this-> _expr-> print (nb + 4);
	    
	if (this-> _type)
	    this-> _type-> print (nb + 4);
    }

		    
}
