#pragma once

#include "Declaration.hh"
#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    class IGlobal : public IDeclaration {

	Word ident;
	Expression expr, type;
	bool isExternal;
	
    public:

	IGlobal (Word ident, Expression expr, Expression type = NULL);

	IGlobal (Word ident, Expression type, bool isExternal);

	void declare () override;
		
	void print (int nb = 0) override;
	
	virtual ~IGlobal ();
	
    };

    typedef IGlobal* Global;
    
}
