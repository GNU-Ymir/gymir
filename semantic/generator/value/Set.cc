#include <ymir/semantic/generator/value/Set.hh>

namespace semantic {

    namespace generator {

	Set::Set () :
	    Value ()
	{}

	Set::Set (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & content) :
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
       
	Generator Set::init (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & values) {
	    return Generator {new (NO_GC) Set (loc, type, values)};
	}

	Generator Set::clone () const {
	    return Generator {new (NO_GC) Set (*this)};
	}

	bool Set::equals (const Generator & gen) const {
	    if (!gen.is <Set> ()) return false;
	    auto block = gen.to<Set> ();
	    if (block._content.size () != this-> _content.size ()) return false;
	    if (!this-> getType ().equals (block.getType ())) return false;

	    for (auto it : Ymir::r (0, this-> _content.size ())) {
		if (!this-> _content [it].equals (block._content [it])) return false;
	    }
	    return true;
	}

	const std::vector <Generator> & Set::getContent () const {
	    return this-> _content;
	}

	std::string Set::prettyString () const {
	    Ymir::OutBuffer buf;
	    for (auto & it : this-> _content) {
		buf.write (it.prettyString (), ";");
	    }
	    return buf.str ();
	}
	
    }
    
}
