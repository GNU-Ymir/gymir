#include <ymir/syntax/expression/Intrinsics.hh>
#include <ymir/syntax/visitor/Keys.hh>

namespace syntax {
    
    Intrinsics::Intrinsics () :
	IExpression (lexing::Word::eof ()),
	_value (Expression::empty ())
    {}
    
    Intrinsics::Intrinsics (const lexing::Word & loc, const Expression & value) :
	IExpression (loc),
	_value (value)
    {}

    Expression Intrinsics::init (const lexing::Word & location, const Expression & value) {
	return Expression {new (NO_GC) Intrinsics (location, value)};
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

    bool Intrinsics::isSizeof () const {
	return this-> getLocation () == Keys::SIZEOF;
    }

    bool Intrinsics::isMove () const {
	return this-> getLocation () == Keys::MOVE;
    }

    bool Intrinsics::isDeepCopy () const {
	return this-> getLocation () == Keys::DCOPY;
    }
    
    std::string Intrinsics::prettyString () const {
	return Ymir::format ("% (%)", this-> getLocation ().getStr (), this-> _value.prettyString ());
    }    

}
