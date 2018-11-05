#pragma once

#include "Declaration.hh"
#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace semantic {
    class ISymbol;
    typedef ISymbol* Symbol;
}

namespace syntax {

    /**
     * \struct IGlobal
     * The syntaxic node representation of a global variable     
     * \verbatim
     global := 'static' identifier (':' type | '=' expression) ';'
     \endverbatim
     */
    class IGlobal : public IDeclaration {

	/** The identifier if the var */
	Word _ident;

	/** The value, may be NULL */
	Expression _expr;

	/** The type, may be NULL */
	Expression _type;

	/** is this var external */
	bool _isExternal;

	/** Is this variable usable as an immutable one */
	bool _isImut = false;

	/** the type information of the variable */
	semantic::Symbol _sym;

	/** The external language */
	std::string _from;

	/** The external space */
	std::string _space;
	
    public:

	/** 
	 * \param ident the identifier of the var
	 * \param docs the related comments 
	 * \param expr the value, could be NULL
	 * \param type the type, could be NULL if expr is not NULL
	 */
	IGlobal (Word ident, const std::string & docs, Expression expr, Expression type = NULL);

	/** 
	 * \param ident the identifier of the var
	 * \param docs the related comments 
	 * \param type the type
	 * \param isExternal 
	 */
	IGlobal (Word ident, const std::string & docs, Expression type, bool isExternal);

	Ymir::json generateDocs ();

	/** 
	 * \return get or set the imutability of this variable
	 */
	bool & isImut ();

	/**
	 * \return is this variable an external C language global variable
	 */
	bool isFromC ();

	void declare () override;
	
	void declare (semantic::Module) override;

	void declareAsExtern (semantic::Module) override;

	Declaration templateDeclReplace (const std::map <std::string, Expression>&) override;

	void setFrom (const std::string & from);

	void setSpace (const std::string & space);
	
	/**
	 * \return the value
	 */
	Expression getExpr ();
	
	void print (int nb = 0) override;
	
	virtual ~IGlobal ();
	
    };

    typedef IGlobal* Global;
    
}
