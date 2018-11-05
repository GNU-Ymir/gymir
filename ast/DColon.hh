#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    /**
     * \struct IDColon
     * The syntaxic representation of a double colon expression
     * \verbatim
     dcolon := expression '::' expression
     \endverbatim
     */
    class IDColon : public IExpression {

	/** The left operand */
	Expression _left;

	/** The right operand */
	Expression _right;

    public:

	/**
	 * \param token the location of the operator
	 * \left the left operand
	 * \right the right operand
	 */
	IDColon (Word token, Expression left, Expression right);

	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;

	/**
	 * \return the left operand
	 */
	Expression getLeft ();

	/**
	 * \return the right operand
	 */
	Expression getRight ();
	
	Ymir::Tree toGeneric () override;

	static const char * id () {
	    return TYPEID (IDColon);
	}
	
	std::vector <std::string> getIds () {
	    auto ret = IExpression::getIds ();
	    ret.push_back (TYPEID (IDColon));
	    return ret;
	}
	
	void print (int nb = 0) override;

	std::string prettyPrint () override; 
	
	virtual ~IDColon ();

    };
    
    typedef IDColon* DColon;
};
