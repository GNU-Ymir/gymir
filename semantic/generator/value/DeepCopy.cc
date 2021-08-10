#include <ymir/semantic/generator/value/DeepCopy.hh>

namespace semantic {

    namespace generator {

	DeepCopy::DeepCopy () :
	    Aliaser (lexing::Word::eof (), Generator::empty (), Generator::empty ())
	{}

	DeepCopy::DeepCopy (const lexing::Word & loc, const Generator & type, const Generator & who) :
	    Aliaser (loc, type, who)
	{}
	
	Generator DeepCopy::init (const lexing::Word & loc, const Generator & type, const Generator & who) {
	    return Generator {new (NO_GC) DeepCopy (loc, type, who)};
	}
    
	Generator DeepCopy::clone () const {
	    return Generator {new (NO_GC) DeepCopy (*this)};
	}

	bool DeepCopy::equals (const Generator & gen) const {
	    if (!gen.is <DeepCopy> ()) return false;
	    auto bin = gen.to<DeepCopy> ();	    
	    return this-> _who.equals (bin._who);
	}

	std::string DeepCopy::prettyString () const {
	    return "dcopy " + this-> _who.prettyString ();
	}
	
    }
    
}
