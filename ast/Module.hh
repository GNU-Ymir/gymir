#pragma once

#include <ymir/ast/Declaration.hh>

namespace syntax {

    class IModDecl : public IDeclaration {

	Word ident;

    public :
	IModDecl (Word ident) :
	    ident (ident)
	{}

	void declare () override;

	void declareAsExtern (semantic::Module) override;
       	
    };

    typedef IModDecl* ModDecl;
    
}
