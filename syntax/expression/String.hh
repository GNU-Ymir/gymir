#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct String
     */
    class String : public IExpression {

	/** The content of the string, the escaping procedure is done in semantic analyses */
	std::string _sequence;

    private :

	friend Expression;

	String ();
	
	String (const lexing::Word & loc);

    public :

	static Expression init (const lexing::Word & token, const std::string & sequence);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;
	
    };    

}
