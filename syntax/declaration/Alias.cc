#include <ymir/syntax/declaration/Alias.hh>

namespace syntax {

    Alias::Alias () : _value (nullptr)
    {}
    
    Declaration Alias::init () {
	return Declaration {new (Z0) Alias ()};
    }

    Declaration Alias::init (const lexing::Word & ident, const Expression & value) {
	Alias * ret = new (Z0) Alias ();
	ret-> _ident = ident;
	ret-> _value = value;
	return Declaration {ret};
    }

    Declaration Alias::init (const Alias & other) {
	Alias * ret = new (Z0) Alias ();
	ret-> _ident = other._ident;
	ret-> _value = other._value;
	return Declaration {ret};
    }

    Declaration Alias::clone () const {
	return Alias::init (*this);
    }

    bool Alias::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Alias thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }	    
    
    void Alias::setName (const lexing::Word & ident) {
	this-> _ident = ident;
    }

    const lexing::Word & Alias::getName () const {
	return this-> _ident;
    }    
    
    void Alias::setValue (const Expression & value) {
	this-> _value = value;
    }

    const Expression & Alias::getValue () const {
	return this-> _value;
    }
    
    void Alias::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*", i, '\t');
	stream.writeln ("<Alias> : ", this-> _ident);
	this-> _value.treePrint (stream, i + 1);
    }
    
}
