#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Pragma
     */
    class Pragma : public IExpression {

	std::vector <Expression> _params;

    private :

	friend Expression;

	Pragma ();
	
	Pragma (const lexing::Word & loc, const std::vector <Expression> & params);

    public :

	static Expression init (const lexing::Word & token, const std::vector <Expression> & params);

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	const std::vector <Expression> & getContent () const;

	std::string prettyString () const override;

    protected :
	
	const std::set <std::string> & computeSubVarNames () override;
    };    

}
