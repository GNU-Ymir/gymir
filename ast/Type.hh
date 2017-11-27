#pragma once

#include "Var.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IType : public IVar {

	semantic::InfoType _type;
	
    public:

	IType (Word token, semantic::InfoType type);

	static const char * id ();
	
	const char * getId () override;

	semantic::InfoType type ();	
	
	void print (int nb = 0) override;	
    };

    typedef IType* Type;
    
}
