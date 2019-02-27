#include <ymir/syntax/expression/String.hh>

namespace syntax {

    String::String () :
	IExpression (lexing::Word::eof ())
    {}
    
    String::String (const lexing::Word & loc) :
	IExpression (loc)
    {}

    Expression String::init (const lexing::Word & location, const std::string & sequence) {
	auto ret = new (Z0) String (location);	
	ret-> _sequence = sequence;
	return Expression {ret};
    }

    Expression String::clone () const {
	return Expression {new String (*this)};
    }

    bool String::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	String thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void String::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<String> ", i, '\t');
	stream.writeln (this-> getLocation (), " ", this-> _sequence);
    }
    
}
