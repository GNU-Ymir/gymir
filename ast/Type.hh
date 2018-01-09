#pragma once

#include "Var.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IType : public IVar {

	semantic::InfoType _type;
	
    public:

	IType (Word token, semantic::InfoType type);
	
	static const char * id () {
	    return TYPEID (IType);
	}

	std::vector <std::string> getIds () override;

	Type asType () override;
	
	semantic::InfoType type ();	
	
	void print (int nb = 0) override;	

	std::string prettyPrint () override;
	
    };

    typedef IType* Type;
    
}
