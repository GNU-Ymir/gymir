#include <ymir/semantic/generator/Type.hh>

namespace semantic {

    namespace generator {

	Type::Type () :
	    IGenerator (lexing::Word::eof (), "")
	{}

	Type::Type (const lexing::Word & loc, const std::string & name) :
	    IGenerator (loc, name)
	{}
	
	Generator Type::clone () const {
	    return Generator{new (Z0) Type ()};
	} 
	
	bool Type::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Type thisType; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisType) [0] == vtable) return true;
	    return IGenerator::isOf (type);	
	}

	bool Type::equals (const Generator &) const {
	    return false;
	}

	std::string Type::typeName () const {
	    return "undef";
	}	
    }
    
}
