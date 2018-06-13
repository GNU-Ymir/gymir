#pragma once

#include "Expression.hh"
#include "Var.hh"
#include "Block.hh"
#include "../errors/_.hh"
#include "../semantic/_.hh"
#include "../syntax/Word.hh"
#include <ymir/utils/Array.hh>

namespace semantic {
    class ILambdaFrame;
    typedef ILambdaFrame* LambdaFrame;
}

namespace syntax {

    class ILambdaFunc : public IExpression {

	std::vector <Var> params;
	Var ret;
	Block block;
	Expression expr;
	ulong id;
	bool _isMoved = false;
	
	std::vector <semantic::Frame> frame; 

	static ulong __nbLambda__;
	
    public:

	ILambdaFunc (Word begin, std::vector<semantic::Frame> frame);
	
	ILambdaFunc (Word begin, std::vector <Var> params, Var type, Block block);
	
	ILambdaFunc (Word begin, std::vector <Var> params, Block block);
	
	ILambdaFunc (Word begin, std::vector <Var> params, Expression ret);
	
	Expression expression () override;	

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;

	bool& isMoved ();
	
	std::vector <Var> & getParams ();

	Ymir::Tree toGeneric () override {
	    return Ymir::Tree ();
	}
	
	Expression getExpr ();

	Block getBlock ();
	
	static ulong getLastNb ();


	virtual std::vector <std::string> getIds () override {
	    auto ids = IExpression::getIds ();
	    ids.push_back (TYPEID (ILambdaFunc));
	    return ids;
	}
	
	std::string prettyPrint () override;

	void print (int nb = 0) override;
	
	virtual ~ILambdaFunc ();
	
    };

    typedef ILambdaFunc* LambdaFunc;
    
}
