#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    /**
     * \struct ICast
     * The syntaxic node representation of a cast expression
     * \verbatim
     cast := 'cast' '!' type '(' expression ')'
     \endverbatim
     */
    class ICast : public IExpression {

	Expression _type;
	Expression _expr;

    public:

	/**
	 * \param begin the location of the cast
	 * \param type the type to cast into
	 * \param expr the expression to cast
	 */
	ICast (Word begin, Expression type, Expression expr);

	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;

	virtual std::vector <std::string> getIds () override {
	    auto ids = IExpression::getIds ();
	    ids.push_back (TYPEID (ICast));
	    return ids;
	}
	
	virtual ~ICast ();
	
    };

    typedef ICast* Cast;    
}
