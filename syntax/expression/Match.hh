#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct Match
     */
    class Match : public IExpression {

	Expression _content;
	
	std::vector <Expression> _matchs;

	std::vector <Expression> _actions;

	bool _isFinal;
	
    private :

	friend Expression;

	Match ();
	
	Match (const lexing::Word & loc, const Expression & content,  const std::vector <Expression> & matchs, const std::vector <Expression> & actions, bool isFinal);

    public :

	static Expression init (const lexing::Word & token, const Expression & content, const std::vector <Expression> & matchs, const std::vector <Expression> & actions, bool isFinal = false);

	Expression clone () const override;

	bool isOf (const IExpression * type) const override;

	/**
	 * \return the element that will be passed to the pattern matching
	 */
	const Expression & getContent () const;

	/**
	 * \return the list of matchers
	 */
	const std::vector <Expression> & getMatchers () const;

	/**
	 * \return the list of actions to perfom in case of successful matchs
	 */
	const std::vector <Expression> & getActions () const;

	
	void treePrint (Ymir::OutBuffer & buf, int i) const override;

	bool isFinal () const;
	
    };    

}
