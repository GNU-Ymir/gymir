#include <ymir/semantic/generator/value/ArrayValue.hh>

namespace semantic {

    namespace generator {

	ArrayValue::ArrayValue () :
	    Value ()
	{}

	ArrayValue::ArrayValue (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & content) :
	    Value (loc, type),
	    _content (content)
	{
	    std::vector <Generator> throwers;
	    for (auto &it : this-> _content) {
		auto &ith = it.getThrowers ();
		throwers.insert (throwers.end (), ith.begin (), ith.end ());
	    }
	    
	    this-> setThrowers (throwers);
	}
	
	Generator ArrayValue::init (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & content) {
	    return Generator {new (NO_GC) ArrayValue (loc, type, content)};
	}
    
	Generator ArrayValue::clone () const {
	    return Generator {new (NO_GC) ArrayValue (*this)};
	}

	bool ArrayValue::equals (const Generator & gen) const {
	    if (!gen.is <ArrayValue> ()) return false;
	    auto bin = gen.to<ArrayValue> ();	    
	    if (bin._content.size () != this-> _content.size ()) return false;
	    for (auto it : Ymir::r (0, this-> _content.size ())) {
		if (!this-> _content [it].equals (bin._content [it]))
		    return false;
	    }
	    return true;
	}

	const std::vector <Generator> & ArrayValue::getContent () const {
	    return this-> _content;
	}

	std::string ArrayValue::prettyString () const {
	    std::vector <std::string> content;
	    for (auto & it : this-> _content)
		content.push_back (it.prettyString ());
	    return Ymir::format ("[%]", content);
	}
    }
    
}
