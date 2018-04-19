#pragma once

#include "Expression.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"

namespace semantic {
    class IInfoType;
    typedef IInfoType* InfoType;
}

namespace syntax {

    class IAffectGeneric : public IExpression {
	Expression left, right;
	bool _addr;
	
    public:

	IAffectGeneric (Word word, Expression left, Expression right, bool addr = false);

	Expression expression () override;

	IExpression* templateExpReplace (const std::map <std::string, Expression>&) override {
	    return this;    
	}
	
	Ymir::Tree toGeneric () override;
	
	void print (int) override {};
	
    };

   
    class IFakeDecl : public IExpression {
	Var left;
	Expression right;
	bool _addr, _const;
	
    public:

	IFakeDecl (Word word, Var left, Expression right, bool _const, bool addr = false);

	Expression expression () override;

	IExpression* templateExpReplace (const std::map <std::string, Expression>&) override {
	    return this;    
	}
		
	void print (int) override {};
	
    };

    

    typedef IFakeDecl* FakeDecl;
    typedef IAffectGeneric* AffectGeneric;
  
}


