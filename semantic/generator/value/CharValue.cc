#include <ymir/semantic/generator/value/CharValue.hh>
#include <ymir/semantic/generator/type/Char.hh>
#include <ymir/utils/Match.hh>

namespace semantic {

    namespace generator {

	CharValue::CharValue () :
	    Value ()
	{}

	CharValue::CharValue (const lexing::Word & loc, const Generator & type, uint32_t value) :
	    Value (loc, type),
	    _value (value)
	{}
       
	Generator CharValue::init (const lexing::Word & loc, const Generator & type, uint32_t value) {
	    return Generator {new (NO_GC) CharValue (loc, type, value)};
	}

	Generator CharValue::clone () const {
	    return Generator {new (NO_GC) CharValue (*this)};
	}

	bool CharValue::equals (const Generator & gen) const {
	    if (!gen.is <CharValue> ()) return false;
	    auto floatValue = gen.to <CharValue> ();
	    return this-> getType ().equals (floatValue.getType ())
		&& this-> _value == floatValue._value;
	}

	uint32_t CharValue::getValue () const {
	    return this-> _value;
	}

	std::string CharValue::prettyString () const {	    
	    if (this-> _value < 255) {
		std::string fmt = "";
		match (this-> getType ()) {
		    of (Char, ch) {
			if (ch.getSize () == 8) fmt = "c8";
		    } fo;
		}

		if (this-> _value == '\n') {
		    return Ymir::format ("'\\n'%", fmt);
		} else if (this-> _value == '\b') {
		    return Ymir::format ("'\\b'%", fmt);
		} else if (this-> _value == '\f') {
		    return Ymir::format ("'\\f'%", fmt);
		} else if (this-> _value == '\v') {
		    return Ymir::format ("'\\v'%", fmt);
		} else if (this-> _value == '\t') {
		    return Ymir::format ("'\\t'%", fmt);
		}

		return Ymir::format ("'%'%", (char) this-> _value, fmt);
	    } else {
		return Ymir::format ("'\\u{%}'", this-> _value);
	    }
	}
	
    }
    
}
