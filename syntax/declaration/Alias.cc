#include <ymir/syntax/declaration/Alias.hh>

namespace syntax {

    Alias::Alias () :
	IDeclaration (lexing::Word::eof ()),
	_value (nullptr)
    {}

    Alias::Alias (const lexing::Word & ident, const Expression & value) :
	IDeclaration (ident),
	_value (value)
    {}
    
    Declaration Alias::init (const lexing::Word & ident, const Expression & value) {
	return Declaration {new (Z0) Alias (ident, value)};
    }

    Declaration Alias::init (const Alias & other) {
	return Declaration {new (Z0) Alias (other)};
    }

    bool Alias::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Alias thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
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
