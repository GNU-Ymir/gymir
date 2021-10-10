#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct KeepVar
     */
    class KeepVar : public IExpression {	
    private :

	friend Expression;

	KeepVar ();
	
	KeepVar (const lexing::Word & loc);

    public :

	static Expression init (const lexing::Word & token);


	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	std::string prettyString () const override;
	
    };    

}
