#include <ymir/semantic/generator/value/OptionValue.hh>

namespace semantic {

    namespace generator {

	OptionValue::OptionValue () :
	    Value (),
	    _content (Generator::empty ()),
	    _success (false)
	{}

	OptionValue::OptionValue (const lexing::Word & loc, const Generator & type, const Generator & content, bool success) :
	    Value (loc, type),
	    _content (content),
	    _success (success)
	{
	    this-> isLvalue (true);
	    this-> setThrowers (this-> _content.getThrowers ());
	}
	
	Generator OptionValue::init (const lexing::Word & loc, const Generator & type, const Generator & content, bool success) {
	    return Generator {new (NO_GC) OptionValue (loc, type, content, success)};
	}
    
	Generator OptionValue::clone () const {
	    return Generator {new (NO_GC) OptionValue (*this)};
	}

	bool OptionValue::equals (const Generator & gen) const {
	    if (!gen.is <OptionValue> ()) return false;
	    auto bin = gen.to<OptionValue> ();
	    if (this-> _success != bin._success) return false;
	    return this-> _content.equals (bin._content);
	}

	const Generator & OptionValue::getContent () const {
	    return this-> _content;
	}

	bool OptionValue::isSuccess () const {
	    return this-> _success;
	}

	std::string OptionValue::prettyString () const {
	    if (this-> _success) {
		return Ymir::format ("Ok (%)", this-> _content.prettyString ());
	    } else {
		return Ymir::format ("Err (%)", this-> _content.prettyString ());
	    }
	}
    }
    
}
