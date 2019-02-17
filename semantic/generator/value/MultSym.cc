#include <ymir/semantic/generator/value/MultSym.hh>
#include <ymir/semantic/generator/type/Void.hh>

namespace semantic {

    namespace generator {

	MultSym::MultSym () :
	    Value ()
	{}

	MultSym::MultSym (const lexing::Word & loc, const std::vector <Generator> & gens) :
	    Value (loc, Void::init (loc)),
	    _gens (gens)
	{}	
	
	Generator MultSym::init (const lexing::Word & loc, const std::vector <Generator> & gens) {
	    return Generator {new MultSym (loc, gens)};
	}
    
	Generator MultSym::clone () const {
	    return Generator {new (Z0) MultSym (*this)};
	}

	bool MultSym::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    MultSym thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool MultSym::equals (const Generator & gen) const {
	    if (!gen.is <MultSym> ()) return false;
	    auto mult = gen.to<MultSym> ();	    
	    if (mult.getGenerators ().size () != this-> _gens.size ()) return false;
	    for (auto it : Ymir::r (0, this-> _gens.size ())) {
		if (!mult.getGenerators () [it].equals (this-> _gens [it])) return false;
	    }
	    return true;
	}

	const std::vector <Generator> & MultSym::getGenerators () const {
	    return this-> _gens;
	}
	
    }
    
}
