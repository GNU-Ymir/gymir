#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Scope
     * A scope is a event that append at the end of a scope 
     * It can be a failure, a success or just a standard exit
     * \verbatim
     scope := 'on' Identifier '=>' expression
     \endverbatim
     */
    class Scope : public IExpression {
    protected:
	
	Expression _content;

    protected :

	friend Expression;

	Scope ();
	
	Scope (const lexing::Word & loc, const Expression & content);

    public :

	/**
	 * \brief Create a new Scope 
	 * \param location the location of the scope
	 * \param content the content of the scope
	 */
	static Expression init (const lexing::Word & location, const Expression & content);

	virtual bool isOf (const IExpression * type) const override;

	bool isExit () const;

	bool isSuccess () const;

	bool isFailure () const;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	const Expression & getContent () const;

	std::string prettyString () const override;
	
    };    
    
}
