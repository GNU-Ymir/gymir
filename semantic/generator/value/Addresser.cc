#include <ymir/semantic/generator/value/Addresser.hh>

namespace semantic {

    namespace generator {

	Addresser::Addresser () :
	    Value (),
	    _who (Generator::empty ())
	{}

	Addresser::Addresser (const lexing::Word & loc, const Generator & type, const Generator & who) :
	    Value (loc, type),
	    _who (who)
	{}
	
	Generator Addresser::init (const lexing::Word & loc, const Generator & type, const Generator & who) {
	    return Generator {new Addresser (loc, type, who)};
	}
    
	Generator Addresser::clone () const {
	    return Generator {new (Z0) Addresser (*this)};
	}

	bool Addresser::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Addresser thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool Addresser::equals (const Generator & gen) const {
	    if (!gen.is <Addresser> ()) return false;
	    auto bin = gen.to<Addresser> ();	    
	    return this-> _who.equals (bin._who);
	}

	const Generator & Addresser::getWho () const {
	    return this-> _who;
	}

	std::string Addresser::prettyString () const {
	    return Ymir::format ("&(%)", this-> _who.prettyString ());
	}
    }
    
}
