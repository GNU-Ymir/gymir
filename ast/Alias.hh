#pragma once

#include "Declaration.hh"
#include "../syntax/Word.hh"
#include "../semantic/_.hh"

namespace syntax {

    class IAlias : public IDeclaration {

	Word _ident;
	Expression _value;

    public :

	IAlias (Word ident, Expression value);

	void declare () override;

	void declare (semantic::Module) override;
	
	void declareAsExtern (semantic::Module mod) override;

	void print (int nb = 0) override;
	
    };

    typedef IAlias* Alias;
    
}

