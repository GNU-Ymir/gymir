#pragma once

#include <ymir/ast/Expression.hh>
#include <ymir/ast/Var.hh>

namespace semantic {
    
    class IInfoType;
    typedef IInfoType* InfoType;
    
    struct DestructSolution {
	long score;
	bool valid;
	bool immutable;
	std::map <std::string, InfoType> caster;
	std::vector <syntax::Var> created;

	DestructSolution (long score, bool valid) :
	    score (score), valid (valid)
	{}

	DestructSolution (long score, bool valid, std::map<std::string, InfoType> & elements) :
	    score (score), valid (valid)
	{
	    this-> caster.swap (elements);
	}

	std::string toString ();	
    };

    class DestructSolver {

	ulong __VAR__ = 1;
	ulong __VALUE__ = 1;

	static DestructSolver __instance__;

    public :

	static DestructSolver & instance ();

	DestructSolution solve (syntax::Expression left, syntax::Expression right);
	
	
    };
    

    
}
