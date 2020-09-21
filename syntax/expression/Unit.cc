#include <ymir/syntax/expression/Unit.hh>

namespace syntax {

    Unit::Unit () :
	IExpression (lexing::Word::eof ())
    {}
    
    Unit::Unit (const lexing::Word & loc) :
	IExpression (loc)
    {}

    Expression Unit::init (const lexing::Word & location) {
	return Expression {new (Z0) Unit (location)};
    }

    bool Unit::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Unit thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void Unit::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Unit>", i, '\t');
    }

    std::string Unit::prettyString () const {
	return ";";
    }
    
}
