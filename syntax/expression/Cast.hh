#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Cast
     * Representation of an binary operator 
     * \verbatim
     binary := expression op(':' expression)? expression 
     \endverbatim 
     */
    class Cast : public IExpression {

	/** The left operand, can be a type or a value */
	Expression _type;

	/** The size of the allocation */
	Expression _content;

    private :

	friend Expression;

	Cast ();
	
	Cast (const lexing::Word & loc, const Expression & type, const Expression & content);

    public :

	static Expression init (const lexing::Word & op, const Expression & type, const Expression &content);

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	const Expression & getType () const;

	const Expression & getContent () const;

	std::string prettyString () const override;
	
    };    

}
