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
	ret-> _names = en._names;
	ret-> _values = en._values;
	ret-> _type = en._type;
	return Declaration {ret};
    }

    Declaration Enum::init (const lexing::Word & ident, const Expression& type, const std::vector<lexing::Word> & names, const std::vector <Expression> & values) {
	auto ret = new (Z0) Enum ();
	ret-> _ident = ident;
	ret-> _type = type;
	ret-> _names = names;
	ret-> _values = values;
	return Declaration {ret};
    }
    
    Declaration Enum::init (const lexing::Word & ident, const std::vector<lexing::Word> & names, const std::vector <Expression> & values) {
	auto ret = new (Z0) Enum ();
	ret-> _ident = ident;
	ret-> _names = names;
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

    void Enum::setIdent (const lexing::Word & ident) {
	this-> _ident = ident;
    }

    void Enum::addValue (const lexing::Word & name, const Expression & value) {
	this-> _names.push_back (name);
	this-> _values.push_back (value);
    }

    void Enum::setType (const Expression & expr) {
	this-> _type = expr;
    }

    void Enum::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*", i, '\t');
	stream.writeln ("<Enum> : ", this-> _ident, ":");
	this-> _type.treePrint (stream, i + 1);
	
	for (auto it : Ymir::r (0, this-> _names.size ())) {
	    stream.writef ("%*", i + 1, '\t');
	    stream.writeln ("<EnumValue> : ", this-> _names [it], ":");
	    this-> _values [it].treePrint (stream, i + 2);
	}
    }
    
}
