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

	/** The suffix, can be eof */
	lexing::Word _suffix;
	
    private :

	friend Expression;
	
	Fixed (const lexing::Word & loc, const lexing::Word & suff);

    public :

	static Expression init (const lexing::Word & token, const lexing::Word & suff);

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	const lexing::Word & getSuffix () const;

	std::string prettyString () const override;
	
    };    

}
