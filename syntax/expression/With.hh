#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     */
    class With : public IExpression {

	/** The parameters of the access */
	std::vector <Expression> _decls;

	Expression _content;
	
    private :

	friend Expression;

	With ();
	
	With (const lexing::Word & loc, const std::vector <Expression> & decls, const Expression & content);

    public :

	static Expression init (const lexing::Word & loc, const std::vector <Expression> & decls, const Expression & content);
	
	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	/**
	 * \return the declaration of the with
	 */
	const std::vector <Expression> & getDecls () const;

	/**
	 * \return the content block of the with
	 */
	const Expression & getContent () const;

	std::string prettyString () const override;
	
    };

}
