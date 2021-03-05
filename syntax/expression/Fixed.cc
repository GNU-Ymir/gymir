#include <ymir/syntax/expression/Fixed.hh>

namespace syntax {
    
    Fixed::Fixed (const lexing::Word & loc, const lexing::Word & suffix) :
	IExpression (loc),
	_suffix (suffix)
    {}

    Expression Fixed::init (const lexing::Word & location, const lexing::Word & suffix) {
	return Expression {new (NO_GC) Fixed (location, suffix)};
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
	    return this-> getLocation ().getStr ();
	else
	    return Ymir::format ("%_%", this-> getLocation ().getStr (), this-> _suffix.getStr ());
    }

    const std::set <std::string> & Fixed::computeSubVarNames () {
	this-> setSubVarNames ({});
	return this-> getSubVarNames ();
    }
    
}
