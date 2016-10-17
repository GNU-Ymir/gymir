#pragma once

#include "Expression.hh"
#include "Var.hh"


namespace Syntax {

    struct VarDecl : Expression {

	VarPtr var;       
	
	VarDecl (Lexical::TokenPtr ptr, VarPtr var) 
	    : Expression (ptr, AstEnums::VAR_DECL),
	      var (var)
	{}

	VarDecl (VarDecl * other) 
	    : Expression (other-> token, AstEnums::VAR_DECL),
	      var (new Var (other-> var-> token)) 
	{}

	virtual ExpressionPtr expression ();

	virtual void print (int nb = 0);		

    };

    typedef VarDecl * VarDeclPtr;

};
