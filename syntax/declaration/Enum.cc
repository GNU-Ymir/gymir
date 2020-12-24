#include <ymir/syntax/declaration/Enum.hh>

namespace syntax {

    Enum::Enum () :
	IDeclaration (lexing::Word::eof ()),
	_type (Expression::empty ())
    {}

    Enum::Enum (const lexing::Word & ident, const Expression& type,  const std::vector <Expression> & values) :
	IDeclaration (ident),
	_values (values),
	_type (type)
    {}
    
    Declaration Enum::init (const lexing::Word & ident, const Expression& type,  const std::vector <Expression> & values) {
	return Declaration {new (NO_GC) Enum (ident, type, values)};
    }
    
    bool Enum::isOf (const IDeclaration * type) const {
	auto vtable = reinterpret_cast <const void* const*> (type) [0];
	Enum thisType; // That's why we cannot implement it for all class
	if (reinterpret_cast <const void* const*> (&thisType) [0] == vtable) return true;
	return IDeclaration::isOf (type);
    }	    

    const Expression & Enum::getType () const {
	return this-> _type;
    }

    const std::vector <Expression> & Enum::getValues () const {
	return this-> _values;
    }
    
    void Enum::treePrint (Ymir::OutBuffer & stream, int i) const {
	stream.writef ("%*", i, '\t');
	stream.writeln ("<Enum> : ", this-> getLocation (), ":");
	this-> _type.treePrint (stream, i + 1);
	
	for (auto & it : this-> _values) {
	    stream.writef ("%*", i + 1, '\t');
	    stream.writeln ("<EnumValue> : ");
	    it.treePrint (stream, i + 2);
	}
    }
    
}
