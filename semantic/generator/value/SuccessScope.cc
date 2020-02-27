#include <ymir/semantic/generator/value/SuccessScope.hh>

namespace semantic {

    namespace generator {

	SuccessScope::SuccessScope () :
	    Value (),
	    _who (Generator::empty ())
	{}

	SuccessScope::SuccessScope (const lexing::Word & loc, const Generator & type, const Generator & who, const std::vector <Generator> & values) :
	    Value (loc, type),
	    _who (who),
	    _values (values)
	{
	    auto wth = this-> _who.getThrowers ();
	    for (auto & it : this-> _values) {
		auto &ith = it.getThrowers ();
		wth.insert (wth.end (), ith.begin (), ith.end ());
	    }

	    this-> setThrowers (wth);
	}
	
	Generator SuccessScope::init (const lexing::Word & loc, const Generator & type, const Generator & who, const std::vector <Generator> & values) {
	    return Generator {new (Z0) SuccessScope (loc, type, who, values)};
	}
    
	Generator SuccessScope::clone () const {
	    return Generator {new (Z0) SuccessScope (*this)};
	}

	bool SuccessScope::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    SuccessScope thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool SuccessScope::equals (const Generator & gen) const {
	    if (!gen.is <SuccessScope> ()) return false;
	    auto bin = gen.to<SuccessScope> ();	    
	    if (!this-> _who.equals (bin._who)) return false;
	    if (bin._values.size () != this-> _values.size ()) return false;
	    for (auto it : Ymir::r (0, this-> _values.size ()))
		if (!bin._values [it].equals (this-> _values [it])) return false;
	    return true;
	}

	const Generator & SuccessScope::getWho () const {
	    return this-> _who;
	}

	const std::vector <Generator> & SuccessScope::getValues () const {
	    return this-> _values;
	}
	
	std::string SuccessScope::prettyString () const {
	    return Ymir::format ("&(%)", this-> _who.prettyString ());
	}
    }
    
}
