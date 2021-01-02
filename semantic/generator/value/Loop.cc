#include <ymir/semantic/generator/value/Loop.hh>

namespace semantic {

    namespace generator {

	Loop::Loop () :
	    Value (),
	    _test (Generator::empty ()),
	    _content (Generator::empty ())
	{}
	
	Loop::Loop (const lexing::Word & loc, const Generator & type, const Generator & test, const Generator & content, bool isDo) :
	    Value (loc, type),
	    _test (test),
	    _content (content),
	    _isDo (isDo)
	{
	    this-> setBreaker (false);
	    auto thrs = this-> _test.getThrowers ();
	    auto &cth = this-> _content.getThrowers ();
	    thrs.insert (thrs.end (), cth.begin (), cth.end ());
	    
	    this-> setThrowers (thrs);
	}
	
	Generator Loop::init (const lexing::Word & loc, const Generator & type, const Generator & test, const Generator & content, bool isDo) {
	    return Generator {new (NO_GC) Loop (loc, type, test, content, isDo)};
	}
    
	Generator Loop::clone () const {
	    return Generator {new (NO_GC) Loop (*this)};
	}

	bool Loop::equals (const Generator & gen) const {
	    if (!gen.is <Loop> ()) return false;
	    auto bin = gen.to<Loop> ();	    
	    return this-> _test.equals (bin._test) &&
		this-> _content.equals (bin._content);
	}

	const Generator & Loop::getTest () const {
	    return this-> _test;
	}

	const Generator & Loop::getContent () const {
	    return this-> _content;
	}

	bool Loop::isDo () const {
	    return this-> _isDo;
	}

	std::string Loop::prettyString () const {
	    if (this-> _test.isEmpty ()) {
		return Ymir::format ("loop %", this-> _content.prettyString ());		
	    } else {
		return Ymir::format ("while (%) %",
				     this-> _test.prettyString (),
				     this-> _content.prettyString ()
		);		
	    }
	}
    }
    
}
