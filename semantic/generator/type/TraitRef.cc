#include <ymir/semantic/generator/type/TraitRef.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/semantic/symbol/Trait.hh>

namespace semantic {
    namespace generator {


	TraitRef::TraitRef () :
	    Type ()
	{}

	TraitRef::TraitRef (const lexing::Word & loc, const Symbol & ref) :
	    Type (loc, loc.getStr ())
	{
	    this-> _ref = ref.getPtr ();
	    if (!this-> _ref.lock ()) { // A structure is complex iif one of its field is complex
		this-> isComplex (
		    true
		);
	    }
	}

	Generator TraitRef::init (const lexing::Word&  loc, const Symbol & ref) {
	    return Generator {new (NO_GC) TraitRef (loc, ref)};
	}

	Generator TraitRef::clone () const {
	    return Generator {new (NO_GC) TraitRef (*this)};
	}

	bool TraitRef::equals (const Generator & gen) const {
	    if (!gen.is<TraitRef> ()) return false;
	    auto str = gen.to <TraitRef> ();
	    return (Symbol {this-> _ref}).equals (Symbol {str._ref});
	}

	bool TraitRef::isRefOf (const Symbol & sym) const {
	    return (Symbol {this-> _ref}).isSameRef (sym);
	}

	Symbol TraitRef::getRef () const {
	    return Symbol {this-> _ref};
	}

	bool TraitRef::needExplicitAlias () const {
	    return true;
	}

	bool TraitRef::containPointers () const {
	    return true;
	}
	
	std::string TraitRef::typeName () const {
	    return Ymir::format ("%::%", (Symbol {this-> _ref}).getRealName (), this-> getLocation ().getStr ());
	}

	std::string TraitRef::getMangledName () const {
	    return Ymir::format ("%::%", (Symbol {this-> _ref}).getMangledName (), this-> getLocation ().getStr ());
	}
	
    }
}
