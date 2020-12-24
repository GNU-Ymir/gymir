#include <ymir/semantic/generator/value/List.hh>

namespace semantic {

    namespace generator {

	List::List () :
	    Value ()
	{}

	List::List (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & content) :
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
	
	Generator List::init (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & content) {
	    return Generator {new (NO_GC) List (loc, type, content)};
	}
    
	Generator List::clone () const {
	    return Generator {new (NO_GC) List (*this)};
	}

	bool List::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    List thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
	}

	bool List::equals (const Generator & gen) const {
	    if (!gen.is <List> ()) return false;
	    auto bin = gen.to<List> ();	    
	    if (bin._content.size () != this-> _content.size ()) return false;
	    for (auto it : Ymir::r (0, this-> _content.size ())) {
		if (!this-> _content [it].equals (bin._content [it]))
		    return false;
	    }
	    return true;
	}

	const std::vector <Generator> & List::getParameters () const {
	    return this-> _content;
	}

	std::string List::prettyString () const {
	    std::vector <std::string> content;
	    for (auto & it : this-> _content)
		content.push_back (it.prettyString ());
	    return Ymir::format ("(%)", content);
	}
    }
    
}
