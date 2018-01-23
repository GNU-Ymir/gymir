#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace syntax {

    struct IMatchPair : public IExpression {
	Expression left, right;

    public:

	IMatchPair (Word token, Expression left, Expression right) :
	    IExpression (token),
	    left (left),
	    right (right) {
	}
	
	void print (int nb = 0) override {
	    printf ("\n%*c<MatchPair> %s",
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	    
	    this-> left-> print (nb + 4);
	    this-> right-> print (nb + 4);
	}

	virtual ~IMatchPair () {
	    delete left;
	    delete right;
	}
	
    };
    
    class IMatch : public IExpression {
	Expression expr;
	std::vector<Expression> values;
	std::vector<Block> block;
	Block default_;
	
	std::vector <Expression> results;
	Expression defaultResult;

    public:
	
	IMatch (Word word, Expression expr, std::vector<Expression> values, std::vector <Block> block, Block def) :
	    IExpression (word),
	    expr (expr),
	    values (values),
	    block (block),
	    default_ (def),
	    defaultResult (NULL)
	{}

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	static const char * id () {
	    return TYPEID (IMatch);
	}
	
	std::vector <std::string> getIds () override {
	    auto ret = IExpression::getIds ();
	    ret.push_back (TYPEID (IMatch));
	    return ret;
	}
	
	void print (int nb = 0) override {
	    printf ("\n%*c<Match> %s", 
		    nb, ' ',
		    this-> token.toString ().c_str ()
	    );
	    this-> expr-> print (nb + 4);
	    
	    for (int i = 0 ; i < (int) this-> values.size () ; i++) {
		this-> values [i]-> print (nb + 8);
		this-> block [i]-> print (nb + 8);
	    }

	    if (this-> default_) this-> default_-> print (nb + 10);
	}	

	virtual ~IMatch () {
	    delete expr;
	    for (auto it : values)
		delete it;
	    for (auto it : block)
		delete it;
	    if (default_) delete default_;

	    for (auto it : results)
		delete it;
	    if (defaultResult) delete defaultResult;
	}
	
    };

    typedef IMatch* Match;
  
}
