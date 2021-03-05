#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     */
    class Set : public IExpression {

	std::vector <Expression> _params;
	
    private :

	friend Expression;

	Set ();
	
	Set (const lexing::Word & loc, const std::vector <Expression> & params);

    public :


	static Expression init (const lexing::Word & loc, const std::vector <Expression> & params);
	
	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	/**
	 * \return the content of the set
	 */
	const std::vector <Expression> & getContent () const;


	std::string prettyString () const override;

    protected :
	
	const std::set <std::string> & computeSubVarNames () override;
	
    };

}
