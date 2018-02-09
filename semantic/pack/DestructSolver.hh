#pragma once

#include <ymir/ast/Expression.hh>
#include <ymir/ast/Var.hh>

namespace syntax {
    class IMatchPair;
    class IBinary;
}

namespace semantic {
    
    class IInfoType;
    typedef IInfoType* InfoType;
    
    struct DestructSolution {
	long score;
	bool valid;
	//bool immutable;
	std::vector <syntax::Expression> caster;
	std::vector <syntax::Var> created;
	syntax::Expression test;
	
	DestructSolution (long score, bool valid) :
	    score (score), valid (valid)
	{}

	DestructSolution (long score, bool valid, std::vector<syntax::Expression> & elements) :
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
	
	DestructSolution solveNormal (syntax::Expression left, syntax::Expression right);

	DestructSolution solvePair (syntax::IMatchPair* left, syntax::Expression right);

	DestructSolution solveBinary (syntax::IBinary* bin, syntax::Expression right);
	
    private :

	bool merge (DestructSolution&, const DestructSolution&, const std::string & op);
	
    };
    

    
}
