#pragma once

namespace Syntax {

    struct Expression : Ast {
	Expression (Lexical::TokenPtr ptr) : Ast (ptr) {}
    };

    struct Var;
    typedef std::tr1::shared_ptr<Var> VarPtr;
    
    struct Var : Expression {
	std::vector<VarPtr> templates;

	Var (Lexical::TokenPtr ptr) : Expression (ptr) {}
	Var (Lexical::TokenPtr ptr, std::vector<VarPtr> templates)
	    : Expression (ptr),
	      templates (templates) {}

	static VarPtr empty () {
	    return VarPtr ();
	}
	
    };
    

};
