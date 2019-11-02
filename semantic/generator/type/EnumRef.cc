#include <ymir/semantic/generator/type/EnumRef.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/semantic/symbol/Enum.hh>
#include <ymir/semantic/generator/value/Enum.hh>

namespace semantic {
    namespace generator {
	
	EnumRef::EnumRef () :
	    Type (),
	    _ref (Symbol::__empty__)
	{}

	EnumRef::EnumRef (const lexing::Word & loc, const Symbol & ref) :
	    Type (loc, loc.str),
	    _ref (ref)
	{}

	Generator EnumRef::init (const lexing::Word&  loc, const Symbol & ref) {
	    return Generator {new (Z0) EnumRef (loc, ref)};
	}

	Generator EnumRef::clone () const {
	    return Generator {new (Z0) EnumRef (*this)};
	}
	
	bool EnumRef::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    EnumRef thisEnum; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisEnum) [0] == vtable) return true;
	    return Type::isOf (type);	
	}

	bool EnumRef::equals (const Generator & gen) const {
	    if (!gen.is<EnumRef> ()) return false;
	    auto str = gen.to <EnumRef> ();
	    return this-> _ref.equals (str._ref);
	}

	bool EnumRef::isRefOf (const Symbol & sym) const {
	    return this-> _ref.isSameRef (sym);
	}
	
	const Symbol & EnumRef::getRef () const {
	    return this-> _ref;
	}
	
	std::string EnumRef::typeName () const {
	    return Ymir::format ("%", this-> _ref.getRealName ());
	}

	std::string EnumRef::getMangledName () const {
	    return Ymir::format ("%", this-> _ref.getMangledName ());
	}
	
    }
}
