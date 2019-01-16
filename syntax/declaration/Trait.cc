#include <ymir/syntax/declaration/Trait.hh>

namespace syntax {

    Trait::Trait () {}

    Declaration Trait::init () {
	return Declaration {new (Z0) Trait ()};
    }

    Declaration Trait::init (const Trait & tr) {
	auto ret = new (Z0) Trait ();
	ret-> _name = tr._name;
	ret-> _inner = tr._inner;
	ret-> _isMixin = tr._isMixin;
	return Declaration {ret};
    }

    Declaration Trait::init (const lexing::Word & name, const std::vector <Declaration> & decls, bool isMixin) {
	auto ret = new (Z0) Trait ();
	ret-> _name = name;
	ret-> _inner = decls;
	ret-> _isMixin = isMixin;
	return Declaration {ret};
    }

    Declaration Trait::clone () const {
	return Trait::init (*this);
    }
    
    bool Trait::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const *> (type) [0];
	Trait thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }	    
    
    void Trait::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*<Trait> ", i, '\t');
	stream.writeln (this-> _name, " mixin : ", this-> _isMixin);

	for (auto & it : this-> _inner)
	    it.treePrint (stream, i + 1);
    }

}
