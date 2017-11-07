#pragma once

#include "Expression.hh"

namespace Syntax {
    
    struct Var;

    typedef Var * VarPtr;
        
    struct Var : Expression {
	std::vector<ExpressionPtr> templates;

	Var (Lexical::TokenPtr ptr, AstEnums::AstEnum type) 
	    : Expression (ptr, type)
	{}
	
	Var (Lexical::TokenPtr ptr)
	    : Expression (ptr, AstEnums::VAR)
	{}
	
	Var (Lexical::TokenPtr ptr, std::vector<ExpressionPtr> templates)
	    : Expression (ptr, AstEnums::VAR),
	      templates (templates)
	{}

	static VarPtr empty () {
	    return VarPtr ();
	}
	
	virtual VarPtr declare (const char*);

	virtual VarPtr expression ();
	       	
	bool isType ();
	
	VarPtr asType ();
	
	virtual void print (int nb = 0);	
	
    };
    
    struct TypedVar : Var {
	VarPtr type;

	TypedVar(Lexical::TokenPtr token, VarPtr type)
	    : Var (token, AstEnums::TYPEDVAR),
	      type (type)
	{}

	virtual VarPtr declare (const char*);
	
	virtual void print (int nb = 0);	
    };

    struct Type : Var {

	Type (Lexical::TokenPtr ptr, Semantic::SymbolPtr info)
	    : Var (ptr, AstEnums::TYPE)
	{
	    this-> sym = info;
	}	
	
    };

};
