#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct String
     */
    class String : public IExpression {

	/** The location of the allocation */
	lexing::Word _location;

	/** The content of the string, the escaping procedure is done in semantic analyses */
	std::string _sequence;

    private :

	friend Expression;

	String ();

    public :

	static Expression init (const String & blk);

	static Expression init (const lexing::Word & token, const std::string & sequence);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
    };    

}
