#include <ymir/syntax/expression/Ignore.hh>

namespace syntax {

    Ignore::Ignore () :
	IExpression (lexing::Word::eof ())
    {}
    
    Ignore::Ignore (const lexing::Word & loc) :
	IExpression (loc)
    {}

    Expression Ignore::init (const lexing::Word & location) {
	return Expression {new (NO_GC) Ignore (location)};
    }

    bool Ignore::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Ignore thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void Ignore::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Ignore>", i, '\t');
    }

    std::string Ignore::prettyString () const {
	return "_";
    }
    
}
