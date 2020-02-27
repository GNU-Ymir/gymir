#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Char
     */
    class Char : public IExpression {

	/** The end of the declaration */
	lexing::Word _end;
	
	/** The content of the char, the escaping procedure is done in semantic analyses */
	lexing::Word _sequence;

	lexing::Word _suffix;
	
    private :

	friend Expression;

	Char ();
	
	Char (const lexing::Word & loc, const lexing::Word & end, const lexing::Word & sequence, const lexing::Word & suffix);

    public :

	static Expression init (const lexing::Word & begin, const lexing::Word & end, const lexing::Word & sequence, const lexing::Word & suffix);


	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	/**
	 * \return the suffix of the char const
	 */
	const lexing::Word & getSuffix () const;
	
	/**
	 * \return the content of the char
	 */
	const lexing::Word & getSequence () const;
	
    };    

}
