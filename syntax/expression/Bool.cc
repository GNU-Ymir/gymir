#include <ymir/syntax/expression/Bool.hh>

namespace syntax {

    Bool::Bool () :
	IExpression (lexing::Word::eof ())
    {}
    
    Bool::Bool (const lexing::Word & loc) :
	IExpression (loc)
    {}

    Expression Bool::init (const lexing::Word & location) {
	return Expression {new (NO_GC) Bool (location)};
    }

    bool Bool::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Bool thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void Bool::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Bool> ", i, '\t');
	stream.writeln (this-> getLocation ());
    }
    

    std::string Bool::prettyString () const {
	return this-> getLocation ().getStr ();
    }
}
