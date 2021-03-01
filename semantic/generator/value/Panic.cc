#include <ymir/semantic/generator/value/Panic.hh>
#include <ymir/semantic/generator/type/Void.hh>

namespace semantic {

    namespace generator {

	Panic::Panic () :
	    Value ()
	{
	    this-> setReturner (true);
	}

	Panic::Panic (const lexing::Word & loc) :
	    Value (loc, Void::init (loc))
	{
	    this-> setReturner (true);
	}
	
	Generator Panic::init (const lexing::Word & loc) {
	    return Generator {new (NO_GC) Panic (loc)};
	}
    
	Generator Panic::clone () const {
	    return Generator {new (NO_GC) Panic (*this)};
	}

	bool Panic::equals (const Generator & gen) const {
	    return gen.is <Panic> ();
	}

	std::string Panic::prettyString () const {
	    return Ymir::format ("#!panic!#");
	}
	
    }
    
}
