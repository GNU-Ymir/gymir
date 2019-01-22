#include <ymir/syntax/declaration/Struct.hh>

namespace syntax {

    Struct::Struct () {}

    Declaration Struct::init () {
	return Declaration {new (Z0) Struct ()};
    }

    Declaration Struct::init (const Struct & str) {
	auto ret = new (Z0) Struct ();
	ret-> _decls = str._decls;
	ret-> _cas = str._cas;
	ret-> _name = str._name;
	return Declaration {ret};
    }

    Declaration Struct::init (const lexing::Word & name, const std::vector <lexing::Word> & attrs, const std::vector <Expression> & types) {
	auto ret = new (Z0) Struct ();
	ret-> _name = name;
	ret-> _cas = attrs;
	ret-> _decls = types;
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

    void Struct::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Struct> ", i, '\t');
	stream.writeln (this-> _name, " @{", this-> _cas, "}");

	for (auto & it : this-> _decls) {
	    it.treePrint (stream, i + 1);
	}
    }

    
    void Struct::addCustomAttribute (const lexing::Word & ca) {
	this-> _cas.push_back (ca);
    }

    const std::vector <lexing::Word> & Struct::getCustomAttributes () const {
	return this-> _cas;
    }
    
    void Struct::setName (const lexing::Word & name) {
	this-> _name = name;
    }

    const lexing::Word & Struct::getName () const {
	return this-> _name;
    }

    
    const std::vector <Expression> & Struct::getDeclarations () const {
	return this-> _decls;
    }

}
