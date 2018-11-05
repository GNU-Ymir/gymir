#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    /**
     * \struct IExpand
     * The syntaxic node representation of an expand 
     * Could be either generated at syntax time and semantic time
     * \verbatim
     expand := 'expand' '(' expression ')' | 
               expression '::' 'expand'
     \endverbatim
     */
    class IExpand : public IExpression {

	/** The expression that will be expanded */	
	Expression _expr;

	/** 
	 * The index of the expansion 
	 * This attribute is used in the case of the expansion of a tuple
	 * Generated at the semantic time 
	 * For example the following expansion : 
	 * \verbatim (1, 2)::expand \endverbatim
	 * Will be translated to an array of expression : 
	 * [a : (1, 2), Expand {_expr : &a, _it : 0}, Expand { _expr : &a, _it : 1}];
	 */
	ulong _it;
	
    public :

	/**
	 * \param begin the location of the expression 
	 * \param expr the expression that will be expanded
	 */
	IExpand (Word begin, Expression expr);

	/**
	 * \param begin the location of the expansion 
	 * \param expr the expression that will be expanded
	 * \param it the index used to expand (cf. _it attribute)
	 */
	IExpand (Word begin, Expression expr, ulong it);
	
	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;

	virtual std::vector <std::string> getIds () override {
	    auto ids = IExpression::getIds ();
	    ids.push_back (TYPEID (IExpand));
	    return ids;
	}
	
	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;

	std::string prettyPrint () override;
	
	virtual ~IExpand ();
	
    };

    typedef IExpand* Expand;
    
}
