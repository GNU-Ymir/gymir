#include <ymir/syntax/expression/Float.hh>

namespace syntax {
    
    Float::Float (const lexing::Word & loc, const lexing::Word & prePart, const lexing::Word & decPart, const lexing::Word & suffix) :
	IExpression (loc),
	_prePart (prePart),
	_decPart (decPart),
	_suffix (suffix)
    {}

    Expression Float::init (const lexing::Word & location, const lexing::Word & prePart, const lexing::Word & decPart, const lexing::Word & suffix) {
	return Expression {new (NO_GC) Float (location, prePart, decPart, suffix)};
    }
    
    void Float::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Float> ", i, '\t');
	stream.writeln (this-> getLocation (), " ", this-> _decPart, " ", this-> _suffix);
    }

    std::string Float::getValue () const {
	if (this-> _decPart.isEof ()) {
	    return this-> _prePart.getStr () + ".0";
	} else if (this-> _prePart.isEof ()) {
	    return "0." + this-> _decPart.getStr ();
	} else {
	    return this-> _prePart.getStr () + "." + this-> _decPart.getStr ();
	}
    }

    const lexing::Word & Float::getSuffix () const {
	return this-> _suffix;
    }

    std::string Float::prettyString () const {
	if (this-> _suffix.isEof ()) {
	    return Ymir::format ("%.%", this-> _prePart.getStr (), this-> _decPart.getStr ());
	} else {
	    return Ymir::format ("%.%%", this-> _prePart.getStr (), this-> _decPart.getStr (), this-> _suffix.getStr ());
	}
    }
    
}
