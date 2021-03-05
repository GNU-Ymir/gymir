#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct OfVar
     */
    class OfVar : public IExpression {

	Expression _right;

	bool _isOver;
	
    private :

	friend Expression;

	OfVar ();
	
	OfVar (const lexing::Word & loc, const Expression & type, bool isOver);

    public :

	static Expression init (const lexing::Word & token, const Expression & type, bool isOver);

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	const Expression & getType () const;

	bool isOver () const;

	std::string prettyString () const override;

    protected :
	
	const std::set <std::string> & computeSubVarNames () override;
	
    };    

}
