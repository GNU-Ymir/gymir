#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Float
     * Representation of an binary operator 
     * \verbatim
     binary := expression op(':' expression)? expression 
     \endverbatim 
     */
    class Float : public IExpression {

	/** The location of the allocation */
	lexing::Word _token;

	/** The decimal part of the float */
	lexing::Word _decPart;
	
    private :

	friend Expression;

	Float ();

    public :

	static Expression init (const Float & blk);

	static Expression init (const lexing::Word & token, const lexing::Word & decPart);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
    };    

}
