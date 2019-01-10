#include <ymir/syntax/declaration/Trait.hh>

namespace syntax {

    Trait::Trait () {}

    Declaration Trait::init () {
	return Declaration {new (Z0) Trait ()};
    }

    Declaration Trait::init (const Trait & tr) {
	auto ret = new (Z0) Trait ();
	ret-> _name = tr._name;
	ret-> _varNames = tr._varNames;
	ret-> _varTypes = tr._varTypes;
	ret-> _protoNames = tr._protoNames;
	ret-> _prototypes = tr._prototypes;
	return Declaration {ret};
    }

    Declaration Trait::init (const lexing::Word & name, const std::vector <lexing::Word> & varNames, const std::vector <Expression> &varTypes, const std::vector <lexing::Word> & protoNames, const std::vector <Function::Prototype> & protos) {
	auto ret = new (Z0) Trait ();
	ret-> _name = name;
	ret-> _varNames = varNames;
	ret-> _varTypes = varTypes;
	ret-> _protoNames = protoNames;
	ret-> _prototypes = protos;
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

}
