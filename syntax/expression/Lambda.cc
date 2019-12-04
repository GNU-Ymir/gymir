#include <ymir/syntax/expression/Lambda.hh>

namespace syntax {

    ulong Lambda::__lastId__ = 0;
    
    Lambda::Lambda () :
	IExpression (lexing::Word::eof ()),
	_proto (Function::Prototype::init ()),
	_content (Expression::empty ())
    {
	this-> _uniqId = __lastId__ + 1;
	__lastId__ ++;
    }
    
    Lambda::Lambda (const lexing::Word & loc) :
	IExpression (loc),
	_proto (Function::Prototype::init ()),
	_content (Expression::empty ())
    {
	this-> _uniqId = __lastId__ + 1;
	__lastId__ ++;
    }

    Expression Lambda::init (const lexing::Word & location, const Function::Prototype & proto, const Expression & content) {
	auto ret = new (Z0) Lambda (location);
	ret-> _proto = proto;
	ret-> _content = content;
	return Expression {ret};
    }


    Expression Lambda::refClosure (const syntax::Expression & expr) {
	Lambda lmbd = expr.to <Lambda> ();
	lmbd._isRefClosure = true;
	return Expression {new (Z0) Lambda (lmbd)};
    }

    Expression Lambda::moveClosure (const syntax::Expression & expr) {
	auto lmbd = expr.to <Lambda> ();
	lmbd._isMoveClosure = true;
	return Expression {new (Z0) Lambda (lmbd)};
    }
    
    Expression Lambda::clone () const {
	return Expression {new Lambda (*this)};
    }

    bool Lambda::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Lambda thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void Lambda::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Lambda>", i, '\t');
	this-> _proto.treePrint (stream, i + 1);
	this-> _content.treePrint (stream, i + 1);
    }

    const syntax::Function::Prototype & Lambda::getPrototype () const {
	return this-> _proto;
    }

    const Expression & Lambda::getContent () const {
	return this-> _content;
    }

    bool Lambda::isMoveClosure () const {
	return this-> _isMoveClosure;
    }

    bool Lambda::isRefClosure () const {
	return this-> _isRefClosure;
    }
    
    ulong Lambda::getUniqId () const {
	return this-> _uniqId;
    }
    
}
