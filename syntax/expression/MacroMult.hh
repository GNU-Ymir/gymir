#pragma once

#include <ymir/syntax/Expression.hh>

namespace syntax {

    /**
     * \struct MacroMult
     */
    class MacroMult : public IExpression {

	lexing::Word _end;
	
	lexing::Word _mult;
	
	std::vector <Expression> _content;
	
    private :

	friend Expression;

	MacroMult ();
	
	MacroMult (const lexing::Word & loc, const lexing::Word & end, const std::vector <Expression> & content, const lexing::Word & mult);

    public :

	static Expression init (const lexing::Word & token, const lexing::Word & end, const std::vector <Expression> & content, const lexing::Word & mult);

	bool isOf (const IExpression * type) const override;

	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	std::string prettyString () const override;

	const std::vector <Expression> & getContent () const;

	const lexing::Word & getMult () const;
	
    };    

}
