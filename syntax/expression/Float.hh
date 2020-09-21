#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Float
     * Representation of an binary operator 
     * \verbatim
     binary := expression op(':' expression)? expression 
     \endverbatim 
     */
    class Float : public IExpression {

	lexing::Word _prePart;
	
	/** The decimal part of the float */
	lexing::Word _decPart;

	lexing::Word _suffix;
	
    private :

	friend Expression;

	Float ();
	
	Float (const lexing::Word & loc, const lexing::Word & prePart, const lexing::Word & decPart, const lexing::Word & suff);

    public :

	static Expression init (const lexing::Word & token, const lexing::Word & prePart, const lexing::Word & decPart, const lexing::Word & suff);


	bool isOf (const IExpression * type) const override;
	
	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	/**
	 * \return the value encoded into a string
	 */
	std::string getValue () const;

	/**
	 * \return the suffix of the float
	 */
	const lexing::Word & getSuffix () const;

	std::string prettyString () const override;
	
    };    

}
