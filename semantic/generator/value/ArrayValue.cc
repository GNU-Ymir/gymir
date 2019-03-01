#include <ymir/semantic/generator/value/ArrayValue.hh>

namespace semantic {

    namespace generator {

	ArrayValue::ArrayValue () :
	    Value ()
	{}

	ArrayValue::ArrayValue (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & content) :
	    Value (loc, type),
	    _content (content)
	{}
	
	Generator ArrayValue::init (const lexing::Word & loc, const Generator & type, const std::vector <Generator> & content) {
	    return Generator {new ArrayValue (loc, type, content)};
	}
    
	Generator ArrayValue::clone () const {
	    return Generator {new (Z0) ArrayValue (*this)};
	}

	bool ArrayValue::isOf (const IGenerator * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    ArrayValue thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return Value::isOf (type);	
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
