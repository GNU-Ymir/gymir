#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Unit
     * Representation of an binary operator 
     * \verbatim
     binary := expression op(':' expression)? expression 
     \endverbatim 
     */
    class Unit : public IExpression {
    private :

	friend Expression;

	Unit ();

	Unit (const lexing::Word & loc);

    public :

	static Expression init (const lexing::Word & token);
	
	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	std::string prettyString () const override;

    protected :
	
	const std::set <std::string> & computeSubVarNames () override;
    };    

}
