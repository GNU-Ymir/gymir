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

	static uint64_t __lastId__;
	
	uint64_t _uniqId;

	bool _isRefClosure = false;

	bool _isMoveClosure = false;
	
    private :

	friend Expression;

	Lambda ();
	
	Lambda (const lexing::Word & loc, const Function::Prototype & proto, const Expression & content);

    public :

	static Expression init (const lexing::Word & op, const Function::Prototype & proto, const Expression & content);

	static Expression refClosure (const syntax::Expression & lmbd);

	static Expression moveClosure (const syntax::Expression & lmbd);	
	
	void treePrint (Ymir::OutBuffer & stream, int32_t i) const override;

	const Function::Prototype & getPrototype () const;

	const Expression & getContent () const;

	bool isRefClosure () const;

	bool isMoveClosure () const;

	uint64_t getUniqId () const;

	std::string prettyString () const override;
	
    };    

}
