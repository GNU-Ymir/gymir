#include <ymir/syntax/expression/Intrinsics.hh>
#include <ymir/syntax/visitor/Keys.hh>

namespace syntax {
    
    Intrinsics::Intrinsics () :
	IExpression (lexing::Word::eof ()),
	_value (Expression::empty ())
    {}
    
    Intrinsics::Intrinsics (const lexing::Word & loc) :
	IExpression (loc),
	_value (Expression::empty ())
    {}

    Expression Intrinsics::init (const lexing::Word & location, const Expression & value) {
	auto ret = new (Z0) Intrinsics (location);
	ret-> _value = value;
	return Expression {ret};
    }

    Expression Intrinsics::clone () const {
	return Expression {new (Z0) Intrinsics (*this)};
    }

    bool Intrinsics::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Intrinsics thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }    

    void Intrinsics::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Intrisics> ", i, '\t');
	stream.writeln (this-> getLocation ());
	this-> _value.treePrint (stream, i + 1);
    }

    const Expression & Intrinsics::getContent () const {
	return this-> _value;
    }
    
    bool Intrinsics::isCopy () const {
	return this-> getLocation () == Keys::COPY;
    }

    bool Intrinsics::isAlias () const {
	return this-> getLocation () == Keys::ALIAS;
    }

    bool Intrinsics::isExpand () const {
	return this-> getLocation () == Keys::EXPAND;
    }

    bool Intrinsics::isTypeof () const {
	return this-> getLocation () == Keys::TYPEOF;
    }
    
}
