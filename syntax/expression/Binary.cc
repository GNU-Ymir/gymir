#include <ymir/syntax/expression/Binary.hh>

namespace syntax {
    
    Binary::Binary (const lexing::Word & loc, const Expression & left, const Expression & right, const Expression & type) :
	IExpression (loc),
	_left (left),
	_right (right),
	_type (type)
    {}

    Expression Binary::init (const lexing::Word & location, const Expression & left, const Expression & right, const Expression & type) {
	return Expression {new (NO_GC) Binary (location, left, right, type)};
    }

    Expression Binary::init (const Binary & other) {
	return Expression {new (NO_GC) Binary (other)};
    }
    
    void Binary::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Binary> ", i, '\t');
	stream.writeln (this-> getLocation ());
	stream.writefln ("%*<Type> ", i + 1, '\t');
	this-> _type.treePrint (stream, i + 2);
	this-> _left.treePrint (stream, i + 1);
	this-> _right.treePrint (stream, i + 1);
    }

    const Expression & Binary::getLeft () const {
	return this-> _left;
    }

    const Expression & Binary::getRight () const {
	return this-> _right;
    }

    const Expression & Binary::getType () const {
	return this-> _type;
    }

    std::string Binary::prettyString () const {
	if (this-> getLocation ().getStr() == "::") {
	    return Ymir::format ("%%%", this-> _left.prettyString (), this-> getLocation ().getStr (), this-> _right.prettyString ());
	} else {
	    return Ymir::format ("(% % %)", this-> _left.prettyString (), this-> getLocation ().getStr (), this-> _right.prettyString ());
	}
    }
    
}
