#include <ymir/syntax/expression/Null.hh>

namespace syntax {

    Null::Null () :
	IExpression (lexing::Word::eof ())
    {}
    
    Null::Null (const lexing::Word & loc) :
	IExpression (loc)
    {}

    Expression Null::init (const lexing::Word & location) {
	return Expression {new (NO_GC) Null (location)};
    }

    bool Null::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Null thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void Null::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Null>", i, '\t');
    }

    std::string Null::prettyString () const {
	return "null";
    }
    
}
