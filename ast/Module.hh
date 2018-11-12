#pragma once

#include <ymir/ast/Declaration.hh>
#include <ymir/ast/Expression.hh>

namespace syntax {

    /**
     * \struct IModDecl
     * Declaration of a module, 
     * \verbatim
     module := 'mod' name ';' 
               | 'mod' Identifier (template)? '{' declaration* '}'

     name := Identifier ('.' Identifier)*
     \endverbatim
     */
    class IModDecl : public IDeclaration {

	/** The identifier of the module */	
	Word _ident;

	/** Is this module global ? */
	bool _isGlobal = true;

	/** The declaration inside the module */
	std::vector <Declaration> _decls;

	/** The templates of the module */
	std::vector <Expression> _tmps;
	
    public :

	/**
	 * \param ident the location and name of the module
	 * \param docs the related comments 
	 */
	IModDecl (Word ident, const std::string & docs) :
	    IDeclaration (docs),
	    _ident (ident),
	    _isGlobal (true)
	{
	    this-> setPublic (true);
	}

	/**
	 * \param ident the location and name of the module
	 * \param docs the related comments 
	 * \param decls the set of declaration inside the module
	 */
	IModDecl (Word ident, const std::string & docs, std::vector <Declaration> & decls) :
	    IDeclaration (docs),
	    _ident (ident),
	    _isGlobal (false),
	    _decls (decls)
	{
	    this-> setPublic (true);
	}

	/**
	 * \return get or set the temlates of the module
	 */
	std::vector <Expression> & templates ();

	/**
	 * \return the templates of the module
	 */
	const std::vector <Expression> & getTemplates ();

	/**
	 * \return the list of declaration inside the module
	 */
	const std::vector <Declaration> & getDecls ();
	
	Declaration templateDeclReplace (const std::map <std::string, Expression>& tmps) override;

	Ymir::json generateDocs () override;
	
	void declare () override;

	void declare (semantic::Module) override;
	
	void declareAsExtern (semantic::Module) override;

	/**
	 * \return get or set the identifier of the module
	 */
	Word & ident () {
	    return this-> _ident;
	}

	/**
	 * \return the identifier of the module
	 */
	const Word & getIdent () {
	    return this-> _ident;
	}

	/**
	 * \return is this module local or global one ? (declared as the module englobing the file)
	 */
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
