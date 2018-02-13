#pragma once

#include <ymir/ast/Declaration.hh>

namespace syntax {

    class IModDecl : public IDeclaration {

	Word ident;
	bool _isGlobal = true;
	std::vector <Declaration> decls;
	
    public :

	IModDecl (Word ident) :
	    ident (ident),
	    _isGlobal (true)
	{
	    this-> is_public (true);
	}

	IModDecl (Word ident, std::vector <Declaration> & decls) :
	    ident (ident),
	    _isGlobal (false),
	    decls (decls)
	{
	    this-> is_public (true);
	}

	void declare () override;

	void declare (semantic::Module) override;
	
	void declareAsExtern (semantic::Module) override;

	Word & getIdent () {
	    return this-> ident;
	}

	bool isGlobal () {
	    return this-> _isGlobal;
	}
	
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
