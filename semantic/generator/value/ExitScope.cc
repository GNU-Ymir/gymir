#include <ymir/semantic/generator/value/ExitScope.hh>

namespace semantic {

    namespace generator {

	ExitScope::ExitScope () :
	    Value (),
	    _who (Generator::empty ()),
	    _jmpBuf (Generator::empty ())
	{}

	ExitScope::ExitScope (const lexing::Word & loc, const Generator & type, const Generator & jmpBuf, const Generator & who, const std::vector <Generator> & values) :
	    Value (loc, type),
	    _who (who),
	    _jmpBuf (jmpBuf),
	    _values (values)
	{}
	
	Generator ExitScope::init (const lexing::Word & loc, const Generator & type, const Generator & jmpBuf, const Generator & who, const std::vector <Generator> & values) {
	    return Generator {new ExitScope (loc, type, jmpBuf, who, values)};
	}
    
	Generator ExitScope::clone () const {
	    return Generator {new (Z0) ExitScope (*this)};
	}

	bool ExitScope::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    ExitScope thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool ExitScope::equals (const Generator & gen) const {
	    if (!gen.is <ExitScope> ()) return false;
	    auto bin = gen.to<ExitScope> ();	    
	    if (!this-> _who.equals (bin._who)) return false;
	    if (bin._values.size () != this-> _values.size ()) return false;
	    for (auto it : Ymir::r (0, this-> _values.size ()))
		if (!bin._values [it].equals (this-> _values [it])) return false;
	    return true;
	}

	const Generator & ExitScope::getWho () const {
	    return this-> _who;
	}

	const std::vector <Generator> & ExitScope::getValues () const {
	    return this-> _values;
	}

	const Generator & ExitScope::getJmpbufType () const {
	    return this-> _jmpBuf;
	}
	
	std::string ExitScope::prettyString () const {
	    return Ymir::format ("&(%)", this-> _who.prettyString ());
	}
    }
    
}
