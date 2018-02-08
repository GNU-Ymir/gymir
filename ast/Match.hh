#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>
#include <ymir/semantic/pack/DestructSolver.hh>

namespace semantic {
    struct DestructSolution;
}

namespace syntax {

    struct IMatchPair : public IExpression {
	Expression left, right;

    public:

	IMatchPair (Word token, Expression left, Expression right);
	
	void print (int nb = 0) override;

	virtual ~IMatchPair ();
	
    };
    
    class IMatch : public IExpression {
	Expression expr;
	std::vector<Expression> values;
	std::vector<Block> block;
	Block default_;
	
	std::vector <Expression> results;
	Expression defaultResult;

	std::vector <semantic::DestructSolution> soluce;
	
    public:

	IMatch (Word word, Expression expr);
	
	IMatch (Word word, Expression expr, std::vector<Expression> values, std::vector <Block> block, Block def);

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;
	
	Expression expression () override;

	static const char * id () {
	    return TYPEID (IMatch);
	}
	
	std::vector <std::string> getIds () override {
	    auto ret = IExpression::getIds ();
	    ret.push_back (TYPEID (IMatch));
	    return ret;
	}
	
	void print (int nb = 0) override;

	virtual ~IMatch ();
	
    };

    typedef IMatch* Match;
  
}
