#include <ymir/semantic/generator/type/TraitRef.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/semantic/symbol/Trait.hh>

namespace semantic {
    namespace generator {


	TraitRef::TraitRef () :
	    Type (),
	    _ref (Symbol::__empty__)
	{}

	TraitRef::TraitRef (const lexing::Word & loc, const Symbol & ref) :
	    Type (loc, loc.str),
	    _ref (ref)
	{
	    if (!this-> _ref.isEmpty ()) { // A structure is complex iif one of its field is complex
		this-> isComplex (
		    true
		);
	    }
	}

	Generator TraitRef::init (const lexing::Word&  loc, const Symbol & ref) {
	    return Generator {new (Z0) TraitRef (loc, ref)};
	}

	Generator TraitRef::clone () const {
	    return Generator {new (Z0) TraitRef (*this)};
	}

	bool TraitRef::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    TraitRef thisTrait; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisTrait) [0] == vtable) return true;
	    return Type::isOf (type);	
	}

	bool TraitRef::equals (const Generator & gen) const {
	    if (!gen.is<TraitRef> ()) return false;
	    auto str = gen.to <TraitRef> ();
	    return this-> _ref.equals (str._ref);
	}

	bool TraitRef::isRefOf (const Symbol & sym) const {
	    return this-> _ref.isSameRef (sym);
	}

	const Symbol & TraitRef::getRef () const {
	    return this-> _ref;
	}

	bool TraitRef::needExplicitAlias () const {
	    return true;
	}
	
	std::string TraitRef::typeName () const {
	    return Ymir::format ("%::%", this-> _ref.getRealName (), this-> getLocation ().str);
	}

	std::string TraitRef::getMangledName () const {
	    return Ymir::format ("%::%", this-> _ref.getMangledName (), this-> getLocation ().str);
	}
	
    }
}
