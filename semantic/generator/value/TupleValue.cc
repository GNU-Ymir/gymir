#include <ymir/semantic/generator/value/TupleValue.hh>

namespace semantic {

    namespace generator {

	TupleValue::TupleValue () :
	    Value ()
	{
	    this-> isLvalue (true);
	}

	TupleValue::TupleValue (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & content) :
	    Value (loc, type),
	    _content (content)
	{
	    this-> isLvalue (true);
	    std::vector <Generator> thrs;
	    for (auto & it : this-> _content) {
		auto &ith = it.getThrowers ();
		thrs.insert (thrs.end (), ith.begin (), ith.end ());
	    }
	    
	    this-> setThrowers (thrs);
	}
	
	Generator TupleValue::init (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & content) {
	    return Generator {new (NO_GC) TupleValue (loc, type, content)};
	}
    
	Generator TupleValue::clone () const {
	    return Generator {new (NO_GC) TupleValue (*this)};
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
