#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace syntax {

    /**
     * \struct IBinary
     * Syntaxic node representation of a binary expression
     * \verbatim
     binary := expression Operator(':' type) expression
     \endverbatim
     */
    class IBinary : public IExpression {
	
	Expression _left, _right;
	Expression _autoCaster;
	
	bool _isRight = false;

    public :

	/**
	 * \param word the location of the operator (the Operator)
	 * \param left the left operand
	 * \param right the right operand
	 * \param ctype the autocaster type
	 */
	IBinary (Word word, Expression left, Expression right, Expression ctype = NULL);

	Expression expression () override;

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;

	/**
	 * \return the left operand
	 */
	Expression& getLeft ();

	/**
	 * \return the right operand
	 */
	Expression& getRight ();

	/**
	 * \return The autocast type
	 */
	Expression& getAutoCast ();
	
	Ymir::Tree toGeneric () override;
	
	void print (int nb = 0) override;

	std::string prettyPrint () override;

	/**
	 * \return true if this is an affectation, false otherwise
	 */
	bool isLvalue () override;
	
	static const char * id () {
	    return TYPEID (IBinary);
	}
	
	std::vector <std::string> getIds () override;

	virtual ~IBinary ();
	
    private:

	/**
	 * \brief semantic analyse of an affectation 
	 * \verbatim
	 left = right
	 \endverbatim
	 */
	Expression affect ();

	/**
	 * \brief semantic analyse of an reaffectation 
	 * \verbatim
	 left ('+=' | '-=' | ...) right
	 \endverbatim
	 */	
	Expression reaff ();

	/**
	 * \brief semantic analyse of a '|' operator  \n
	 * It can return a type if both left and right are types
	 */
	Expression bitwiseOr ();

	/**
	 * \brief semantic analyse of all the remaining operators
	 */
	Expression normal (IBinary* = NULL);

	/**
	 * \brief Common verification of the validity of the left and right operand
	 * \warning this method is called in affect, reaff bitwiseOr and normal methods
	 */
	bool simpleVerif (IBinary*);

	/**
	 * \brief Check if the operator is capable of overidde the assign operator	 
	 */
	bool canOverOpAssign (IBinary*);

	/**
	 * \brief Check if the operator is capable of overidde the binary operator	 
	 */
	bool canOverOpBinary (IBinary*);

	/**
	 * \brief Search a \a opBinary method overriding the operator 
	 */
	Expression findOpBinary (IBinary*);

	/**
	 * \brief Search a \a opAssign method overriding the operator 
	 */
	Expression findOpAssign (IBinary*, bool mandatory = true);

	/**
	 * \brief Search a \a opTest method overriding the operator 
	 */
	Expression findOpTest (IBinary*);

	/**
	 * \brief Search a \a opEqual method overriding the operator 
	 */
	Expression findOpEqual (IBinary*);

	/**
	   \return true if and only if the operator return a boolean type
	 */
	bool isTest (Word);

	std::string oppositeTest (Word);
	bool isEq (Word);
	
    };

    typedef IBinary* Binary;
    
}
