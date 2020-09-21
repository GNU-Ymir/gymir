#include <ymir/syntax/expression/Float.hh>

namespace syntax {

    Float::Float () :
	IExpression (lexing::Word::eof ())
    {}
    
    Float::Float (const lexing::Word & loc, const lexing::Word & prePart, const lexing::Word & decPart, const lexing::Word & suffix) :
	IExpression (loc),
	_prePart (prePart),
	_decPart (decPart),
	_suffix (suffix)
    {}

    Expression Float::init (const lexing::Word & location, const lexing::Word & prePart, const lexing::Word & decPart, const lexing::Word & suffix) {
	return Expression {new (Z0) Float (location, prePart, decPart, suffix)};
    }
    
    bool Float::isOf (const IExpression * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Float thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IExpression::isOf (type);
    }

    void Float::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Float> ", i, '\t');
	stream.writeln (this-> getLocation (), " ", this-> _decPart, " ", this-> _suffix);
    }

    std::string Float::getValue () const {
	if (this-> _decPart.isEof ()) {
	    return this-> _prePart.str + ".0";
	} else if (this-> _prePart.isEof ()) {
	    return "0." + this-> _decPart.str;
	} else {
	    return this-> _prePart.str + "." + this-> _decPart.str;
	}
    }

    const lexing::Word & Float::getSuffix () const {
	return this-> _suffix;
    }

    std::string Float::prettyString () const {
	if (this-> _suffix.isEof ()) {
	    return Ymir::format ("%.%", this-> _prePart.str, this-> _decPart.str);
	} else {
	    return Ymir::format ("%.%%", this-> _prePart.str, this-> _decPart.str, this-> _suffix.str);
	}
    }
    
}
