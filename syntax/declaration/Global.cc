#include <ymir/syntax/declaration/Global.hh>

namespace syntax {


    Global::Global () :
	_decl (Expression::empty ())
    {}

    Declaration Global::init () {
	return Declaration {new (Z0) Global ()};
    }

    Declaration Global::init (const Global & gl) {
	auto ret = new (Z0) Global ();
	ret-> _location = gl._location;
	ret-> _decl = gl._decl;
	return Declaration {ret};
    }

    Declaration Global::init (const lexing::Word & location, const Expression & decl) {
	auto ret = new (Z0) Global ();
	ret-> _location = location;
	ret-> _decl = decl;
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

    const Expression & Global::getContent () const {
	return this-> _decl;
    }
    
    const lexing::Word & Global::getLocation () const {
	return this-> _location;
    }

    void Global::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Global>", i, '\t');
	stream.writeln (" ", this-> _location);       
	this-> _decl.treePrint (stream, i + 2);
    }
        
}
