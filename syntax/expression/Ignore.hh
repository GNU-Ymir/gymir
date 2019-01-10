#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Ignore
     * Representation of an binary operator 
     * \verbatim
     binary := expression op(':' expression)? expression 
     \endverbatim 
     */
    class Ignore : public IExpression {

	/** The location of the allocation */
	lexing::Word _token;

    private :

	friend Expression;

	Ignore ();

    public :

	static Expression init (const Ignore & blk);

	static Expression init (const lexing::Word & token);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
    };    

}
