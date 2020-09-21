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
    private :

	friend Expression;

	Ignore ();

	Ignore (const lexing::Word & loc);

    public :

	static Expression init (const lexing::Word & token);

	bool isOf (const IExpression * type) const override;
	
	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	std::string prettyString () const override;

    };    

}
