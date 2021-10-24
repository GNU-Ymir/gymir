#include <ymir/semantic/generator/value/Conditional.hh>

namespace semantic {

    namespace generator {

	Conditional::Conditional () :
	    Value (),
	    _test (Generator::empty ()),
	    _content (Generator::empty ()),
	    _else (Generator::empty ())
	{}
	
	Conditional::Conditional (const lexing::Word & loc, const Generator & type, const Generator & test, const Generator & content, const Generator & else_, bool isMandatory) :
	    Value (loc, type),
	    _test (test),
	    _content (content),
	    _else (else_),
	    _isComplete (isMandatory)
	{
	    if (!else_.isEmpty ()) {
		this-> setBreaker (
		    this-> _content.to <Value> ().isBreaker () &&
		    this-> _else.to <Value> ().isBreaker ()
		);
		
		if (this-> _content.to<Value> ().isBreaker ())
		    this-> setBreakerLocation (this-> _content.to <Value> ().getBreakerLocation ());
		else if (this-> _else.to<Value> ().isBreaker ())
		    this-> setBreakerLocation (this-> _else.to <Value> ().getBreakerLocation ());
		
	    	this-> setReturner (
		    this-> _content.to <Value> ().isReturner () &&
		    this-> _else.to <Value> ().isReturner ()
		);

		if (this-> _content.to <Value> ().isReturner ())
		    this-> setReturnerLocation (this-> _content.to <Value> ().getReturnerLocation ());
		else if (this-> _else.to <Value> ().isReturner ())
		    this-> setReturnerLocation (this-> _else.to <Value> ().getReturnerLocation ());
		
		this-> _isComplete = !this-> _else.is<Conditional> () ||
		    this-> _else.to <Conditional> ().isComplete ();
		
	    } else if (isMandatory) {
		this-> setBreaker (
		    this-> _content.to <Value> ().isBreaker ()
		);
		
		if (this-> _content.to<Value> ().isBreaker ())
		    this-> setBreakerLocation (this-> _content.to <Value> ().getBreakerLocation ());
				
	    	this-> setReturner (
		    this-> _content.to <Value> ().isReturner ()
		);
		
		if (this-> _content.to <Value> ().isReturner ())
		    this-> setReturnerLocation (this-> _content.to <Value> ().getReturnerLocation ());
	    }

	    auto lth = this-> _test.getThrowers ();
	    auto &cth = this-> _content.getThrowers ();
	    auto &eth = this-> _else.getThrowers ();
	    
	    lth.insert (lth.end (), cth.begin (), cth.end ());
	    lth.insert (lth.end (), eth.begin (), eth.end ());

	    this-> setThrowers (lth);	    
	}	
	
	Generator Conditional::init (const lexing::Word & loc, const Generator & type, const Generator & test, const Generator & content, const Generator & else_, bool isMandatory) {
	    return Generator {new (NO_GC) Conditional (loc, type, test, content, else_, isMandatory)};
	}
    
	Generator Conditional::clone () const {
	    return Generator {new (NO_GC) Conditional (*this)};
	}

	bool Conditional::equals (const Generator & gen) const {
	    if (!gen.is <Conditional> ()) return false;
	    auto bin = gen.to<Conditional> ();	    
	    return this-> _test.equals (bin._test) &&
		this-> _content.equals (bin._content) &&
		this-> _else.equals (bin._else);
	}

	const Generator & Conditional::getTest () const {
	    return this-> _test;
	}

	const Generator & Conditional::getContent () const {
	    return this-> _content;
	}
	
	const Generator & Conditional::getElse () const {
	    return this-> _else;
	}

	std::string Conditional::prettyString () const {
	    if (this-> _else.isEmpty ()) 
		return Ymir::format ("if (%) %", this-> _test.prettyString (), this-> _content.prettyString ());
	    else
		return Ymir::format ("if (%) %\nelse %", this-> _test.prettyString (), this-> _content.prettyString (), this-> _else.prettyString ());
	}

	
	bool Conditional::isComplete () const {
	    return this-> _isComplete;
	}
	
    }
    
}
