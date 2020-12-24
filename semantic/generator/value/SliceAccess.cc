#include <ymir/semantic/generator/value/SliceAccess.hh>

namespace semantic {

    namespace generator {

	SliceAccess::SliceAccess () :
	    Value (),
	    _slice (Generator::empty ()),
	    _index (Generator::empty ())
	{
	    this-> isLvalue (true);
	}

	SliceAccess::SliceAccess (const lexing::Word & loc, const Generator & type, const Generator & slice, const Generator & index) :
	    Value (loc, type),
	    _slice (slice),
	    _index (index)
	{
	    this-> isLvalue (true);
	    auto sth = this-> _slice.getThrowers ();
	    auto &ith = this-> _index.getThrowers ();
	    sth.insert (sth.end (), ith.begin (), ith.end ());
	    
	    this-> setThrowers (sth);
	}
	
	Generator SliceAccess::init (const lexing::Word & loc, const Generator & type, const Generator & slice, const Generator & index) {
	    return Generator {new (NO_GC) SliceAccess (loc, type, slice, index)};
	}
    
	Generator SliceAccess::clone () const {
	    return Generator {new (NO_GC) SliceAccess (*this)};
	}

	bool SliceAccess::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    SliceAccess thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool SliceAccess::equals (const Generator & gen) const {
	    if (!gen.is <SliceAccess> ()) return false;
	    auto bin = gen.to<SliceAccess> ();	    
	    return bin._slice.equals (this-> _slice) && bin._index.equals (this-> _index);
	}

	const Generator & SliceAccess::getSlice () const {
	    return this-> _slice;
	}

	const Generator & SliceAccess::getIndex () const {
	    return this-> _index;
	}

	std::string SliceAccess::prettyString () const {
	    return Ymir::format ("% [%]", this-> _slice.prettyString (), this-> _index.prettyString ());
	}
    }
    
}
