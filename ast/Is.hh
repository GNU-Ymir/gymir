#pragma once

#include <ymir/utils/Array.hh>
#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    /**
     * \struct IIs
     * The syntaxic node representation of a is expression
     * \verbatim
     is := 'is' '(' expression ':' (expression (',' template_param)*) | 'fn' | 'tuple' | 'struct' | 'type' ')'
     template_param := Constante | Identifier 'of' expression | Identifier ':' expression | Identifier 
     \endverbatim
     */
    class IIs : public IExpression {
	
	/** The template parameters */
	std::vector <Expression> _tmps;

	/** The left operand */
	Expression _left;

	/** The type (right operand), may be NULL */
	Expression _type;

	/** The type (right operand), equals to Word::eof (), iif _type != NULL */
	Word _expType;

    public:

	/**
	 * \param begin the location of this expression
	 * \param expr the expression to evaluated
	 * \param type the type we expect to test 
	 * \param tmps the templates parameters
	 */
	IIs (Word begin, Expression expr, Expression type, std::vector <Expression> tmps);

	/**
	 * \param begin the location of this expression
	 * \param expr the expression to evaluated
	 * \param type the type we expect to test 
	 */
	IIs (Word begin, Expression expr, Word type);
	
	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;

	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;

	virtual std::vector <std::string> getIds () override {
	    auto ids = IExpression::getIds ();
	    ids.push_back (TYPEID (IIs));
	    return ids;
	}
	
	std::string prettyPrint () override;
	
	virtual ~IIs ();
	
    };

    typedef IIs* Is;
   
}
