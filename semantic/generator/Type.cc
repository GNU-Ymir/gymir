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

	bool Type::isCompatible (const Generator & gen) const {
	    return this-> equals (gen);
	}
	
	std::string Type::typeName () const {
	    return "undef";
	}	

	bool Type::isRef () const {
	    return this-> _isRef;
	}

	bool Type::isMutable () const {
	    return this-> _isMutable;
	}

	bool Type::isComplex () const {
	    return this-> _isComplex;
	}
	
	void Type::isRef (bool is) {
	    this-> _isRef = is;
	}
	
	void Type::isMutable (bool is) {
	    this-> _isMutable = is;
	}

	void Type::isComplex (bool is) {
	    this-> _isComplex = is;
	}
	
    }
    
}
