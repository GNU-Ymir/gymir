#include <ymir/syntax/expression/VariadicVar.hh>

namespace syntax {

    VariadicVar::VariadicVar () :
	IExpression (lexing::Word::eof ())
    {}
    
    VariadicVar::VariadicVar (const lexing::Word & loc, bool isValue) :
	IExpression (loc),
	_isValue (isValue)
    {}
    
    Expression VariadicVar::init (const lexing::Word & location, bool isValue) {
	return Expression {new (NO_GC) VariadicVar (location, isValue)};
    }

    bool VariadicVar::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	VariadicVar thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }


    void VariadicVar::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<VariadicVar> ", i, '\t');
	stream.writeln (this-> getLocation (), ' ', this-> _isValue);
    }

    std::string VariadicVar::prettyString () const {
	if (this-> _isValue)
	    return Ymir::format ("%:...", this-> getLocation ().getStr ());
	else
	    return Ymir::format ("%...", this-> getLocation ().getStr ());
    }
}
