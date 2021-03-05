#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Null
     */
    class Null : public IExpression {
    private :

	friend Expression;

	Null ();
	
	Null (const lexing::Word & loc);

    public :

	static Expression init (const lexing::Word & token);

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	std::string prettyString () const override;

    protected :
	
	const std::set <std::string> & computeSubVarNames () override;
	
    };    

}
