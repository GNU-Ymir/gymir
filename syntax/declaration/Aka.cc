#include <ymir/syntax/declaration/Aka.hh>

namespace syntax {

    Aka::Aka () :
	IDeclaration (lexing::Word::eof (), ""),
	_value (nullptr)
    {}

    Aka::Aka (const lexing::Word & ident, const std::string & comment, const Expression & value) :
	IDeclaration (ident, comment),
	_value (value)
    {}
    
    Declaration Aka::init (const lexing::Word & ident, const std::string & comment, const Expression & value) {
	return Declaration {new (NO_GC) Aka (ident, comment, value)};
    }

    Declaration Aka::init (const Aka & other) {
	return Declaration {new (NO_GC) Aka (other)};
    }
    
    
    const Expression & Aka::getValue () const {
	return this-> _value;
    }
    
    void Aka::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*", i, '\t');
	stream.writeln ("<Aka> : ", this-> getLocation ());
	this-> _value.treePrint (stream, i + 1);
    }
    
}
