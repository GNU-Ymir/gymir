#include <ymir/semantic/generator/value/Move.hh>

namespace semantic {

    namespace generator {

	Move::Move () :
	    Aliaser (lexing::Word::eof (), Generator::empty (), Generator::empty ()), _replace (Generator::empty ())
	{}

	Move::Move (const lexing::Word & loc, const Generator & type, const Generator & who, const Generator & replace) :
	    Aliaser (loc, type, who), _replace (replace)
	{}
	
	Generator Move::init (const lexing::Word & loc, const Generator & type, const Generator & who, const Generator & replace) {
	    return Generator {new (NO_GC) Move (loc, type, who, replace)};
	}
    
	Generator Move::clone () const {
	    return Generator {new (NO_GC) Move (*this)};
	}

	bool Move::equals (const Generator & gen) const {
	    if (!gen.is <Move> ()) return false;
	    auto bin = gen.to<Move> ();	    
	    return this-> _who.equals (bin._who);
	}


	const Generator & Move::getReplace () const {
	    return this-> _replace;
	}
	
	std::string Move::prettyString () const {
	    return "move " + this-> _who.prettyString ();
	}
	
    }
    
}
