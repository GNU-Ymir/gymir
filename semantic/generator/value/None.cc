#include <ymir/semantic/generator/value/None.hh>
#include <ymir/semantic/generator/type/Void.hh>

namespace semantic {

    namespace generator {

	None::None () :
	    Value ()
	{}

	None::None (const lexing::Word & loc) :
	    Value (loc, Void::init (loc))
	{}
       
	Generator None::init (const lexing::Word & loc) {
	    return Generator {new (NO_GC) None (loc)};
	}

	Generator None::clone () const {
	    return Generator {new (NO_GC) None (*this)};
	}

	bool None::equals (const Generator & gen) const {
	    return gen.is <None> ();
	    
	}
	
    }
    
}
