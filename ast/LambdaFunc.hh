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

    /**
     * \struct ILambdaFunc
     * The syntaxic node representation of a lambda function
     */
    class ILambdaFunc : public IExpression {

	/** The parameters of the function */
	std::vector <Var> _params;

	/** The return type of the function, may be NULL */
	Var _ret;

	/** The block of instruction, is NULL iif _expr != NULL */
	Block _block;

	/** The result expression of the function */
	Expression _expr;

	/** The uniq id of this lambda function */
	ulong _id;

	/** True if this function has a moved closure (false means the closure vars are by reference) */
	bool _isMoved = true;

	/** The associated frames */
	std::vector <semantic::Frame> _frame; 

	/** The number of declared lambda function */
	static ulong __nbLambda__;
	
    public:

	ILambdaFunc (Word begin, std::vector<semantic::Frame> frame);
	
	ILambdaFunc (Word begin, std::vector <Var> params, Var type, Block block);
	
	ILambdaFunc (Word begin, std::vector <Var> params, Block block);
	
	ILambdaFunc (Word begin, std::vector <Var> params, Expression ret);
	
	Expression expression () override;	

	Expression templateExpReplace (const std::map <std::string, Expression>&) override;

	bool& isMoved ();
	
	const std::vector <Var> & getParams ();

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
