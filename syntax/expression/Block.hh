#pragma once

#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/Declaration.hh>

namespace syntax {

    /**
     * \struct Block
     * Representation of an binary operator 
     * \verbatim
     binary := expression op(':' expression)? expression 
     \endverbatim 
     */
    class Block : public IExpression {

	/** The end of the block */
	lexing::Word _end;

	/** The content of the block */
	std::vector <Expression> _content;

	/** The declaration done inside the block */
	std::vector <Declaration> _decls;
	
    private :

	friend Expression;

	Block ();
	
	Block (const lexing::Word & loc);

    public :

	static Expression init (const lexing::Word & op, const lexing::Word & end, const std::vector <Declaration> & decls, const std::vector <Expression> & conetent);

	Expression clone () const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	bool isOf (const IExpression * type) const override;

	/**
	 * \return the list of expression the block contains
	 */
	const std::vector <Expression> & getContent () const;

	/**
	 * \return the closing token of the block
	 */
	const lexing::Word & getEnd () const;
	
    };    

}
