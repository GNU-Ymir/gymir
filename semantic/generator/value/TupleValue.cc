#include <ymir/semantic/generator/value/TupleValue.hh>

namespace semantic {

    namespace generator {

	TupleValue::TupleValue () :
	    Value ()
	{}

	TupleValue::TupleValue (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & content) :
	    Value (loc, type),
	    _content (content)
	{
	    std::vector <Generator> thrs;
	    for (auto & it : this-> _content) {
		auto &ith = it.getThrowers ();
		thrs.insert (thrs.end (), ith.begin (), ith.end ());
	    }
	    
	    this-> setThrowers (thrs);
	}
	
	Generator TupleValue::init (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & content) {
	    return Generator {new (Z0) TupleValue (loc, type, content)};
	}
    
	Generator TupleValue::clone () const {
	    return Generator {new (Z0) TupleValue (*this)};
	}

	bool TupleValue::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    TupleValue thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool TupleValue::equals (const Generator & gen) const {
	    if (!gen.is <TupleValue> ()) return false;
	    auto bin = gen.to<TupleValue> ();	    
	    if (bin._content.size () != this-> _content.size ()) return false;
	    for (auto it : Ymir::r (0, this-> _content.size ())) {
		if (!this-> _content [it].equals (bin._content [it]))
		    return false;
	    }
	    return true;
	}

	const std::vector <Generator> & TupleValue::getContent () const {
	    return this-> _content;
	}

	std::string TupleValue::prettyString () const {
	    std::vector <std::string> content;
	    for (auto & it : this-> _content)
		content.push_back (it.prettyString ());
	    return Ymir::format ("(%)", content);
	}
    }
    
}
