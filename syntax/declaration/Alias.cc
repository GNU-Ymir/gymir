#include <ymir/syntax/declaration/Alias.hh>

namespace syntax {

    Alias::Alias () :
	IDeclaration (lexing::Word::eof (), ""),
	_value (nullptr)
    {}

    Alias::Alias (const lexing::Word & ident, const std::string & comment, const Expression & value) :
	IDeclaration (ident, comment),
	_value (value)
    {}
    
    Declaration Alias::init (const lexing::Word & ident, const std::string & comment, const Expression & value) {
	return Declaration {new (NO_GC) Alias (ident, comment, value)};
    }

    Declaration Alias::init (const Alias & other) {
	return Declaration {new (NO_GC) Alias (other)};
    }
    
    
    const Expression & Alias::getValue () const {
	return this-> _value;
    }
    
    void Alias::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*", i, '\t');
	stream.writeln ("<Alias> : ", this-> getLocation ());
	this-> _value.treePrint (stream, i + 1);
    }
    
}
