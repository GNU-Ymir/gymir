#pragma once

#include "Declaration.hh"
#include "../syntax/Word.hh"
#include "../semantic/_.hh"

namespace syntax {

    /**
     * \struct IAlias
     * This class represent the syntax node of an alias declaration
     * \verbatim
     alias := 'alias' Identifier '=' expression ';'
     \endverbatim
     */
    class IAlias : public IDeclaration {

	Word _ident;
	Expression _value;

    public :

	/**
	 * \param ident the identifier of the alias
	 * \param value the value of the alias
	 */
	IAlias (Word ident, Expression value);

	void declare () override;

	void declare (semantic::Module) override;
	
	void declareAsExtern (semantic::Module mod) override;

	void print (int nb = 0) override;
	
    };

    typedef IAlias* Alias;
    
}

