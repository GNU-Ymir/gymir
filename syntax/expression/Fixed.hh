#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Fixed
     * Representation of an binary operator 
     * \verbatim
     binary := expression op(':' expression)? expression 
     \endverbatim 
     */
    class Fixed : public IExpression {

	/** The location of the allocation */
	lexing::Word _token;

	/** The suffix, can be eof */
	lexing::Word _suffix;
	
    private :

	friend Expression;

	Fixed ();

    public :

	static Expression init (const Fixed & blk);

	static Expression init (const lexing::Word & token, const lexing::Word & suff);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;
    };    

}
