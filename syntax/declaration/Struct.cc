#include <ymir/syntax/declaration/Struct.hh>

namespace syntax {

    Struct::Struct () {}

    Declaration Struct::init () {
	return Declaration {new (Z0) Struct ()};
    }

    Declaration Struct::init (const Struct & str) {
	auto ret = new (Z0) Struct ();
	ret-> _varNames = str._varNames;
	ret-> _varTypes = str._varTypes;
	ret-> _cas = str._cas;
	ret-> _name = str._name;
	return Declaration {ret};
    }

    Declaration Struct::init (const lexing::Word & name, const std::vector <lexing::Word> & names, const std::vector <Expression> & types) {
	auto ret = new (Z0) Struct ();
	ret-> _name = name;
	ret-> _varNames = names;
	ret-> _varTypes = types;
	return Declaration {ret};
    }

    Declaration Struct::clone () const {
	return Struct::init (*this);
    }

    bool Struct::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Struct thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }	    

    void Struct::addCustomAttribute (const lexing::Word & ca) {
	this-> _cas.push_back (ca);
    }

    void Struct::setName (const lexing::Word & name) {
	this-> _name = name;
    }

    void Struct::addAttribute (const lexing::Word & name, const Expression & type) {
	this-> _varNames.push_back (name);
	this-> _varTypes.push_back (type);
    }
    
}
