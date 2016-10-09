#pragma once

#include "TypeInfo.hh"

namespace Semantic {

    struct IntInfo : TypeInfo {

	IntInfo ();
	
	IntInfo (TypeInfo::BinopLint lint);

	IntInfo (TypeInfo::UnopLint lint);
	
	TypeInfo * binaryOp (Lexical::TokenPtr, Syntax::Expression*) override;
	
	TypeInfo * binaryOpRight (Lexical::TokenPtr tok, Syntax::Expression*) override;
	
	TypeInfo * unaryOp (Lexical::TokenPtr tok) override;
	
	TypeInfo * multOp (Lexical::TokenPtr tok, Syntax::Expression*) override;

	TypeInfo * clone () override;
		
	bool Is (TypeEnum) override;

	std::string typeToString () const;
	
	static TypeInfo* create (std::vector<Syntax::Expression*>) {
	    return new IntInfo ();
	}

    private:

	//Constructeur static
	static class _init {
	public :
	    _init () {
		TypeInfo::creators["int"] = &IntInfo::create;
	    }
	} _initializer;

    private:
	
	TypeInfo * Affect (TypeInfo*);
	TypeInfo * PlusAff (TypeInfo*);
	TypeInfo * SubAff (TypeInfo*);
	TypeInfo * MulAff (TypeInfo*);	
	TypeInfo * DivAff (TypeInfo*);
	TypeInfo * Inf (TypeInfo*);
	TypeInfo * Sup (TypeInfo*);
	TypeInfo * Equals (TypeInfo*);
	TypeInfo * SupEquals (TypeInfo*);
	TypeInfo * InfEquals (TypeInfo*);
	TypeInfo * NotEquals (TypeInfo*);
	TypeInfo * Plus (TypeInfo*);
	TypeInfo * Sub (TypeInfo*);
	TypeInfo * OrBit (TypeInfo*);
	TypeInfo * LeftBit (TypeInfo*);
	TypeInfo * RightBit (TypeInfo*);
	TypeInfo * Xor (TypeInfo*);
	TypeInfo * Mul (TypeInfo*);
	TypeInfo * Div (TypeInfo*);
	TypeInfo * AndBit (TypeInfo*);
	TypeInfo * Modulo (TypeInfo*);	
    };
    

}
