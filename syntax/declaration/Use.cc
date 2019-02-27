#include <ymir/syntax/declaration/Use.hh>

namespace syntax {

    Use::Use () : _module (Expression::empty ())
    {}

    Declaration Use::init () {
	return Declaration {new (Z0) Use ()};
    }

    Declaration Use::init (const Use & use) {
	auto ret = new (Z0) Use ();
	ret-> _location = use._location;
	ret-> _module = use._module;
	return Declaration {ret};
    }

    Declaration Use::init (const lexing::Word & loc, const Expression & module) {
	auto ret = new (Z0) Use ();
	ret-> _module = module;
	ret-> _location = loc;
	return Declaration {ret};
    }

    Declaration Use::clone () const {
	return Use::init (*this);
    }

    bool Use::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Use thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }

    void Use::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writefln ("%*<Use> ", i, '\t');
	this-> _module.treePrint (stream, i + 1);
    }
    
}
