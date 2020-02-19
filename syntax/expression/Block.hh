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

	/** The declaration done inside the block */
	Declaration _declModule;

	Expression _catcher;

	/** The content of the block */
	std::vector <Expression> _content;

	/** The scope guard of the block */
	std::vector <Expression> _scopes;
	
    private :

	friend Expression;

	Block ();
	
	Block (const lexing::Word & loc);

    public :

	static Expression init (const lexing::Word & op, const lexing::Word & end, const Declaration & decls, const std::vector <Expression> & content, const Expression & catcher, const std::vector <Expression> & scopes);

	Expression clone () const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	bool isOf (const IExpression * type) const override;

	/**
	 * \return the list of expression the block contains
	 */
	const std::vector <Expression> & getContent () const;

	/**
	 * \return the list of declaration of the block
	 */
	const Declaration & getDeclModule () const;
	
	/**
	 * \return the closing token of the block
	 */
	const lexing::Word & getEnd () const;

	/**
	 * \return the catcher of this block, might be empty
	 */
	const Expression & getCatcher () const;

	/**
	 * \return the list of scope guards of the block
	 */
	const std::vector <Expression> & getScopes () const;
	
	
    };    

}
