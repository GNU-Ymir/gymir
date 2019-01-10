#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Pragma
     */
    class Pragma : public IExpression {

	/** The location of the allocation */
	lexing::Word _token;

	std::vector <Expression> _params;

    private :

	friend Expression;

	Pragma ();

    public :

	static Expression init (const Pragma & blk);

	static Expression init (const lexing::Word & token, const std::vector <Expression> & params);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
    };    

}
