#pragma once

#include <ymir/ast/Declaration.hh>
#include <ymir/ast/Expression.hh>

namespace syntax {

    class IModDecl : public IDeclaration {

	Word ident;
	bool _isGlobal = true;
	std::vector <Declaration> decls;
	std::vector <Expression> tmps;
	
    public :

	IModDecl (Word ident, const std::string & docs) :
	    IDeclaration (docs),
	    ident (ident),
	    _isGlobal (true)
	{
	    this-> setPublic (true);
	}

	IModDecl (Word ident, const std::string & docs, std::vector <Declaration> & decls) :
	    IDeclaration (docs),
	    ident (ident),
	    _isGlobal (false),
	    decls (decls)
	{
	    this-> setPublic (true);
	}

	std::vector <Expression> & getTemplates ();

	std::vector <Declaration> & getDecls ();
	
	Declaration templateDeclReplace (const std::map <std::string, Expression>& tmps);

	Ymir::json generateDocs () override;
	
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
