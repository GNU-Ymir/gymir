#include <ymir/semantic/generator/type/StructRef.hh>
#include <ymir/utils/OutBuffer.hh>

namespace semantic {
    namespace generator {
	
	StructRef::StructRef () :
	    Type (),
	    _ref (Symbol::__empty__)
	{}

	StructRef::StructRef (const lexing::Word & loc, const Symbol & ref) :
	    Type (loc, loc.str),
	    _ref (ref)
	{}

	Generator StructRef::init (const lexing::Word&  loc, const Symbol & ref) {
	    return Generator {new (Z0) StructRef (loc, ref)};
	}

	Generator StructRef::clone () const {
	    return Generator {new (Z0) StructRef (*this)};
	}

	bool StructRef::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    StructRef thisStruct; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisStruct) [0] == vtable) return true;
	    return Type::isOf (type);	
	}

	bool StructRef::equals (const Generator & gen) const {
	    if (!gen.is<StructRef> ()) return false;
	    auto str = gen.to <StructRef> ();
	    return this-> _ref.isSameRef (str._ref);
	}

	bool StructRef::isRefOf (const Symbol & sym) const {
	    return this-> _ref.isSameRef (sym);
	}

	const Symbol & StructRef::getRef () const {
	    return this-> _ref;
	}
	
	std::string StructRef::typeName () const {
	    return Ymir::format ("%", this-> _ref.getRealName ());
	}       
	
    }
}
