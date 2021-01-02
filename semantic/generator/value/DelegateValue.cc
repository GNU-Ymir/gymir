#include <ymir/semantic/generator/value/DelegateValue.hh>

namespace semantic {

    namespace generator {

	DelegateValue::DelegateValue () :
	    Value (),
	    _closureType (Generator::empty ()),
	    _closure (Generator::empty ()),
	    _funcptr (Generator::empty ())
	{}

	DelegateValue::DelegateValue (const lexing::Word & loc, const Generator & type, const Generator & closureType, const Generator & closure, const Generator & ptr) : 
	    Value (loc, type),
	    _closureType (closureType),
	    _closure (closure),	    
	    _funcptr (ptr)
	{
	    
	    auto thrs = this-> getType ().to <Type> ().getInners ()[0].getThrowers ();
	    for (auto & it : thrs) it = Generator::init (loc, it);	    

	    if (this-> getType ().to <Type> ().getInners () [0].is <Type> ()) { // If it is a delegate to a template value, then the throwers are already declared
		auto & fptrT = this-> _funcptr.getThrowers ();
		thrs.insert (thrs.end (), fptrT.begin (), fptrT.end ());
	    }
	    
	    this-> setThrowers (thrs);
	}
       
	Generator DelegateValue::init (const lexing::Word & loc, const Generator & type, const Generator & closureType, const Generator & closure, const Generator & ptr) {
	    return Generator {new (NO_GC) DelegateValue (loc, type, closureType, closure, ptr)};
	}

	Generator DelegateValue::clone () const {
	    return Generator {new (NO_GC) DelegateValue (*this)};
	}

	bool DelegateValue::equals (const Generator & gen) const {
	    if (!gen.is <DelegateValue> ()) return false;
	    auto floatValue = gen.to <DelegateValue> ();
	    return this-> getType ().equals (floatValue.getType ())
		&& this-> _funcptr.equals (floatValue._funcptr) && this-> _closure.equals (floatValue._closure);
	}

	const Generator & DelegateValue::getClosure () const {
	    return this-> _closure;
	}

	const Generator & DelegateValue::getFuncPtr () const {
	    return this-> _funcptr;
	}

	const Generator & DelegateValue::getClosureType () const {
	    return this-> _closureType;
	}
	
	std::string DelegateValue::prettyString () const {
	    if (this-> _closure.is <Value> ()) {
		return Ymir::format ("{% = % : %, %} ",
				     this-> _closureType.prettyString (),
				     this-> _closure.prettyString (),
				     this-> _closure.to <Value> ().getType ().prettyString (),
				     this-> _funcptr.prettyString ()
		);
	    } else {
		return Ymir::format ("{% = %, %} ",
				     this-> _closureType.prettyString (),
				     this-> _closure.prettyString (),
				     this-> _funcptr.prettyString ()
		);
	    }
	}
	
    }
    
}
