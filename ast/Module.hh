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

	static const char * id () {
	    return TYPEID (IModDecl);
	}
	
	std::vector <std::string> getIds () override {
	    return {TYPEID (IModDecl)};
	}

	void print (int) override {}
	
    };

    typedef IModDecl* ModDecl;
    
}
