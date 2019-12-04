#pragma once

#include <ymir/syntax/Expression.hh>
#include <ymir/syntax/declaration/Function.hh>

namespace syntax {

    /**
     * \struct Lambda
     */
    class Lambda : public IExpression {

	Function::Prototype _proto;
	
	/** The content of the function */
	Expression _content;

	static ulong __lastId__;
	
	ulong _uniqId;

	bool _isRefClosure = false;

	bool _isMoveClosure = false;
	
    private :

	friend Expression;

	Lambda ();
	
	Lambda (const lexing::Word & loc);

    public :

	static Expression init (const lexing::Word & op, const Function::Prototype & proto, const Expression & content);

	static Expression refClosure (const syntax::Expression & lmbd);

	static Expression moveClosure (const syntax::Expression & lmbd);
	
	Expression clone () const override;

	bool isOf (const IExpression * type) const override;
	
	void treePrint (Ymir::OutBuffer & stream, int i) const override;

	const Function::Prototype & getPrototype () const;

	const Expression & getContent () const;

	bool isRefClosure () const;

	bool isMoveClosure () const;

	ulong getUniqId () const;

	std::string prettyString () const override;
	
    };    

}
