#pragma once

#include "Var.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class ITypedVar : public IVar {

	Var type;
	Expression expType;

    public:

	ITypedVar (Word ident, Var type);

	ITypedVar (Word ident, Var type, Word deco);

	ITypedVar (Word ident, Expression type);

	ITypedVar (Word ident, Expression type, Word deco);
	
	static const char * id ();

	const char * getId () override;

	semantic::InfoType getType ();

	Expression expression ();

	Var var () override;
	
	void print (int nb = 0) override;

	std::string prettyPrint () override;
    };

    typedef ITypedVar* TypedVar;
    
}
