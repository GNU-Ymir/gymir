#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Char
     */
    class Char : public IExpression {

	/** The content of the char, the escaping procedure is done in semantic analyses */
	std::string _sequence;

    private :

	friend Expression;

	Char ();
	
	Char (const lexing::Word & loc);

    public :

	static Expression init (const lexing::Word & token, const std::string & sequence);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
    };    

}
