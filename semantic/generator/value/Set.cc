#include <ymir/semantic/generator/value/Set.hh>

namespace semantic {

    namespace generator {

	Set::Set () :
	    Value ()
	{}

	Set::Set (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & content) :
	    Value (loc, type),
	    _content (content)
	{}
       
	Generator Set::init (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & values) {
	    return Generator {new (Z0) Set (loc, type, values)};
	}

	Generator Set::clone () const {
	    return Generator {new (Z0) Set (*this)};
	}

	bool Set::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    Set thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
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

	void Set::setContent (const std::vector<Generator> & content) {
	    this-> _content = content;
	}
	
    }
    
}