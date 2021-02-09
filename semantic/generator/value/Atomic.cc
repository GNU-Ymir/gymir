#include <ymir/semantic/generator/value/Atomic.hh>
#include <ymir/semantic/generator/type/NoneType.hh>

namespace semantic {

    namespace generator {

	AtomicLocker::AtomicLocker () :
	    Value (),
	    _who (Generator::empty ())
	{}

	AtomicLocker::AtomicLocker (const lexing::Word & loc, const Generator & who, bool isMonitor) :
	    Value (loc, NoneType::init (loc)),	    
	    _who (who),
	    _isMonitor (isMonitor)
	{
	    this-> setThrowers (this-> _who.getThrowers ());
	}
	
	Generator AtomicLocker::init (const lexing::Word & loc, const Generator & who, bool isMonitor) {
	    return Generator {new (NO_GC) AtomicLocker (loc, who, isMonitor)};
	}
    
	Generator AtomicLocker::clone () const {
	    return Generator {new (NO_GC) AtomicLocker (*this)};
	}

	bool AtomicLocker::equals (const Generator & gen) const {
	    if (!gen.is <AtomicLocker> ()) return false;
	    auto bin = gen.to<AtomicLocker> ();	    
	    return this-> _who.equals (bin._who);
	}

	const Generator & AtomicLocker::getWho () const {
	    return this-> _who;
	}

	bool AtomicLocker::isMonitor () const {
	    return this-> _isMonitor;
	}
	
	std::string AtomicLocker::prettyString () const {
	    return Ymir::format ("__lock (%)", this-> getType ().to <Type> ().getTypeName (), this-> _who.prettyString ());
	}
   

	AtomicUnlocker::AtomicUnlocker () :
	    Value (),
	    _who (Generator::empty ())
	{}

	AtomicUnlocker::AtomicUnlocker (const lexing::Word & loc, const Generator & who, bool isMonitor) :
	    Value (loc, NoneType::init (loc)),	    
	    _who (who),
	    _isMonitor (isMonitor)
	{
	    this-> setThrowers (this-> _who.getThrowers ());
	}
	
	Generator AtomicUnlocker::init (const lexing::Word & loc, const Generator & who, bool isMonitor) {
	    return Generator {new (NO_GC) AtomicUnlocker (loc, who, isMonitor)};
	}
    
	Generator AtomicUnlocker::clone () const {
	    return Generator {new (NO_GC) AtomicUnlocker (*this)};
	}

	bool AtomicUnlocker::equals (const Generator & gen) const {
	    if (!gen.is <AtomicUnlocker> ()) return false;
	    auto bin = gen.to<AtomicUnlocker> ();	    
	    return this-> _who.equals (bin._who);
	}

	const Generator & AtomicUnlocker::getWho () const {
	    return this-> _who;
	}

	bool AtomicUnlocker::isMonitor () const {
	    return this-> _isMonitor;
	}

	std::string AtomicUnlocker::prettyString () const {
	    return Ymir::format ("__unlock (%)", this-> getType ().to <Type> ().getTypeName (), this-> _who.prettyString ());
	}
    
    }
}
