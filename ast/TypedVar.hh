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
	
	static const char * id () {
	    return TYPEID (ITypedVar);
	}	
	std::vector <std::string> getIds () override;

	semantic::InfoType getType ();

	Var typeVar ();
	
	Expression typeExp ();
	
	Expression expression ();

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;	
	
	Var var () override;
	
	void print (int nb = 0) override;

	std::string prettyPrint () override;

	virtual ~ITypedVar ();
	
    };

    typedef ITypedVar* TypedVar;
    
}
