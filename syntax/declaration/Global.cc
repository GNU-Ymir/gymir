#include <ymir/syntax/declaration/Global.hh>

namespace syntax {


    Global::Global () :
	_value (Expression::empty ()),
	_type (Expression::empty ())
    {}

    Declaration Global::init () {
	return Declaration {new (Z0) Global ()};
    }

    Declaration Global::init (const Global & gl) {
	auto ret = new (Z0) Global ();
	ret-> _ident = gl._ident;
	ret-> _value = gl._value;
	ret-> _type = gl._type;
	return Declaration {ret};
    }

    Declaration Global::init (const lexing::Word & ident, const Expression & type, const Expression & value) {
	auto ret = new (Z0) Global ();
	ret-> _ident = ident;
	ret-> _type = type;
	ret-> _value = value;
	return Declaration {ret};
    }

    Declaration Global::clone () const {
	return Global::init (*this);
    }

    bool Global::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Global thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }

    void Global::setName (const lexing::Word & name) {
	this-> _ident = name;
    }

    void Global::setValue (const Expression & value) {
	this-> _value = value;
    }

    void Global::setType (const Expression & type) {
	this-> _type = type;
    }
    
}
