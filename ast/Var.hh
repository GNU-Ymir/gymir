#pragma once

#include "Expression.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

#include <vector>

namespace syntax {

    class ITypedVar;
    typedef ITypedVar* TypedVar;

    class IType;
    typedef IType* Type;
    
    class IVar : public IExpression {
    protected :
	
	std::vector <Expression> templates;

    public :

	Word deco;

	IVar (Word ident);
	IVar (Word ident, Word deco);
	IVar (Word ident, std::vector <Expression> tmps);

	static const char* id ();
	
	const char* getId () override;
	
	bool hasTemplate ();

	Expression expression () override;

	IVar* var ();

	Type asType ();

	bool isType ();

	std::vector<Expression>& getTemplates ();

	Word& getDeco ();
	
	TypedVar setType (::semantic::Symbol info);

	TypedVar setType (::semantic::InfoType info);

	void print (int nb = 0) override;
    };

    class IArrayVar : public IVar {
	Expression content;

    public: 
	IArrayVar (Word token, Expression content) :
	    IVar (token),
	    content (content)
	{}
	
	void print (int nb = 0) override {
	    printf ("\n%*c <ArrayVar> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
		    
	}
	
    };
    
    typedef IVar* Var;    
    typedef IArrayVar* ArrayVar;
}
