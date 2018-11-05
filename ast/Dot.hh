#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    /**
     * \struct
     * The syntaxic node representation of a Dot expression
     * \verbatim
     dot := expression '.' expression
     \endverbatim
     */
    class IDot : public IExpression {

	/** The left operand */
	Expression _left;

	/** The right operand */
	Expression _right;

	/** In the case of dotcall, it's not necessary to reproduce the semantic analyse*/
	bool _isDone = false;
	
    public:

	/**
	 * \param word the location of the expression 
	 * \param left the left operand
	 * \param right the right operand
	 */
	IDot (Word word, Expression left, Expression right);
	
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

	std::string prettyPrint () override;

	/**
	 * \return true
	 */
	bool isLvalue () override;
	
	static const char * id () {
	    return TYPEID (IDot);
	}
	
	std::vector <std::string> getIds () {
	    auto ret = IExpression::getIds ();
	    ret.push_back (TYPEID (IDot));
	    return ret;
	}
		
	void print (int nb = 0) override;

	virtual ~IDot ();
	
    };

    /**
     * \struct IDotCall
     * Only used at semantic time, represent a dot call, for uniform call convention 
     * Example : 
     * \verbatim
def foo (a : X) { ... }

let x = X {};
x.foo ();
     \endverbatim
     */
    class IDotCall : public IExpression {
    private:

	/** The call expression */
	Expression _call;

	/** The first parameter */
	Expression _firstPar;

	/** is this call a dynamic one ? */
	bool _dyn;
	
    public:

	/**
	 * \param inside the instruction in wich this call is 
	 * \param token the location of this expression
	 * \param the call expression 
	 * \param firstPar the firstParameter of the call
	 */
	IDotCall (Instruction inside, Word token, Expression call, Expression firstPar);

	Expression expression () override;

	/**
	 * \return get or set the call attribute of the expression
	 */
	Expression& call ();

	/**
	 * \return get or set the first parameter attribute of the expression
	 */
	Expression& firstPar ();

	static const char * id () {
	    return TYPEID (IDotCall);
	}
	
	std::vector <std::string> getIds ();
	
	void print (int nb = 0) override;

	virtual ~IDotCall ();
	
    };

    
    typedef IDot* Dot;
}
