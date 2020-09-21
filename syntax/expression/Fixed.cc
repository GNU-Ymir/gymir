#include <ymir/syntax/expression/Fixed.hh>

namespace syntax {

    Fixed::Fixed () :
	IExpression (lexing::Word::eof ())
    {}
    
    Fixed::Fixed (const lexing::Word & loc, const lexing::Word & suffix) :
	IExpression (loc),
	_suffix (suffix)
    {}

    Expression Fixed::init (const lexing::Word & location, const lexing::Word & suffix) {
	return Expression {new (Z0) Fixed (location, suffix)};
    }
 

    bool Fixed::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Fixed thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void Fixed::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Fixed> ", i, '\t');
	stream.writeln (this-> getLocation (), " ", this-> _suffix);
    }
    

    const lexing::Word & Fixed::getSuffix () const {
	return this-> _suffix;
    }

    std::string Fixed::prettyString () const {
	if (this-> _suffix.isEof ())
	    return this-> getLocation ().str;
	else
	    return Ymir::format ("%_%", this-> getLocation ().str, this-> _suffix.str);
    }

}
