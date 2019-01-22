#include <ymir/syntax/declaration/Enum.hh>

namespace syntax {

    Enum::Enum () : _type (Expression::empty ())
    {}
    
    Declaration Enum::init () {
	return Declaration {new (Z0) Enum ()};
    }

    Declaration Enum::init (const Enum & en) {
	auto ret = new (Z0) Enum ();
	ret-> _ident = en._ident;
	ret-> _values = en._values;
	ret-> _type = en._type;
	return Declaration {ret};
    }

    Declaration Enum::init (const lexing::Word & ident, const Expression& type,  const std::vector <Expression> & values) {
	auto ret = new (Z0) Enum ();
	ret-> _ident = ident;
	ret-> _type = type;
	ret-> _values = values;
	return Declaration {ret};
    }
    
    Declaration Enum::init (const lexing::Word & ident, const std::vector <Expression> & values) {
	auto ret = new (Z0) Enum ();
	ret-> _ident = ident;
	ret-> _values = values;
	return Declaration {ret};
    }

    Declaration Enum::clone () const {
	return Enum::init (*this);
    }

    bool Enum::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const*> (type) [0];
	Enum thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const*> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }	    

    void Enum::setName (const lexing::Word & ident) {
	this-> _ident = ident;
    }

    const lexing::Word & Enum::getName () const {
	return this-> _ident;
    }
    
    void Enum::addValue (const Expression & value) {
	this-> _values.push_back (value);
    }

    void Enum::setType (const Expression & expr) {
	this-> _type = expr;
    }

    const Expression & Enum::getType () const {
	return this-> _type;
    }

    const std::vector <Expression> & Enum::getValues () const {
	return this-> _values;
    }
    
    void Enum::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*", i, '\t');
	stream.writeln ("<Enum> : ", this-> _ident, ":");
	this-> _type.treePrint (stream, i + 1);
	
	for (auto & it : this-> _values) {
	    stream.writef ("%*", i + 1, '\t');
	    stream.writeln ("<EnumValue> : ");
	    it.treePrint (stream, i + 2);
	}
    }
    
}
