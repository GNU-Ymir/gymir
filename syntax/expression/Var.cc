#include <ymir/syntax/expression/Var.hh>

namespace syntax {

    Var::Var () :
	IExpression (lexing::Word::eof ())
    {}
    
    Var::Var (const lexing::Word & loc) :
	IExpression (loc)
    {}
    
    Expression Var::init (const lexing::Word & location) {
	return Expression {new (Z0) Var (location)};
    }
    
    bool Var::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Var thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void Var::treePrint (Ymir::OutBuffer & stream, int i) const {	
	stream.writef ("%*<Var> : ", i, '\t');
	stream.writeln (this-> getLocation ());
    }

    const lexing::Word & Var::getName () const {
	return this-> getLocation ();
    }

    std::string Var::prettyString () const {
	return this-> getLocation ().str;
    }
    
}
