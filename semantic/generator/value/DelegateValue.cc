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
	    for (auto & it : thrs) it.changeLocation (loc);
	    
	    auto & fptrT = this-> _funcptr.getThrowers ();
	    thrs.insert (thrs.end (), fptrT.begin (), fptrT.end ());
	    
	    this-> setThrowers (thrs);
	}
       
	Generator DelegateValue::init (const lexing::Word & loc, const Generator & type, const Generator & closureType, const Generator & closure, const Generator & ptr) {
	    return Generator {new (Z0) DelegateValue (loc, type, closureType, closure, ptr)};
	}

	Generator DelegateValue::clone () const {
	    return Generator {new (Z0) DelegateValue (*this)};
	}

	bool DelegateValue::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    DelegateValue thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
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
	    return Ymir::format ("{%, %}", this-> _closure.prettyString (), this-> _funcptr.prettyString ());
	}
	
    }
    
}
