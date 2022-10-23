#include <ymir/semantic/generator/value/MultSym.hh>
#include <ymir/semantic/generator/type/NoneType.hh>
#include <ymir/semantic/validator/CallVisitor.hh>
#include <ymir/errors/Error.hh>
#include <ymir/utils/Match.hh>
#include <algorithm>

namespace semantic {

    namespace generator {

	MultSym::MultSym () :
	    Value ()
	{}

	MultSym::MultSym (const lexing::Word & loc, const std::vector <Generator> & gens, bool crashIfZero) :
	    Value (loc, NoneType::init (loc)),
	    _gens (gens)
	{
	    // MultSym are temporary elements, used only at validation time, they cant be throwers
	    for (auto & it : this-> _gens) {
		if (it.isEmpty ()) {
		    Ymir::Error::halt ("", "");
		}
	    }
	    
	    if (crashIfZero && this-> _gens.size () == 0) Ymir::Error::halt ("", "");
	}	
	
	Generator MultSym::init (const lexing::Word & loc, const std::vector <Generator> & gens, bool crashIfZero) {
	    return Generator {new (NO_GC) MultSym (loc, gens, crashIfZero)};
	}
    
	Generator MultSym::clone () const {
	    return Generator {new (NO_GC) MultSym (*this)};
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
	
	std::string MultSym::prettyString () const {
	    if (this-> _gens.size () != 1) {
		Ymir::OutBuffer buf;
		buf.write ("{");
		for (auto it : Ymir::r (0, std::min ((uint32_t) this-> _gens.size (), (uint32_t) 3))) {
		    if (it != 0) buf.write (", ");
		    if (it == 2 && this-> _gens.size () > 3) buf.write ("..., ");
		    buf.write (validator::CallVisitor::prettyName (this-> _gens [it]));
		}
		
		buf.writef ("} x %", (uint32_t) this-> _gens.size ());
		return buf.str ();
	    } else {
		return this-> _gens [0].prettyString ();
	    }
	}
    }
    
}
