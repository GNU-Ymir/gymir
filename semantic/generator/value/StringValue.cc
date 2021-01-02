#include <ymir/semantic/generator/value/StringValue.hh>
#include <ymir/semantic/generator/type/Char.hh>
#include <ymir/semantic/validator/UtfVisitor.hh>

namespace semantic {

    namespace generator {

	StringValue::StringValue () :
	    Value ()
	{}

	StringValue::StringValue (const lexing::Word & loc, const Generator & type, const std::vector <char> &  value, ulong len) :
	    Value (loc, type),
	    _value (value),
	    _len (len)
	{}
       
	Generator StringValue::init (const lexing::Word & loc, const Generator & type, const std::vector<char> &  value, ulong len) {
	    return Generator {new (NO_GC) StringValue (loc, type, value, len)};
	}

	Generator StringValue::clone () const {
	    return Generator {new (NO_GC) StringValue (*this)};
	}

	bool StringValue::equals (const Generator & gen) const {
	    if (!gen.is <StringValue> ()) return false;
	    auto floatValue = gen.to <StringValue> ();
	    return this-> getType ().equals (floatValue.getType ())
		&& this-> _value == floatValue._value;
	}

	ulong StringValue::getLen  () const {
	    return this-> _len;
	}
	
	const std::vector<char> &  StringValue::getValue () const {
	    return this-> _value;
	}

	std::string StringValue::prettyString () const {
	    if (this-> _value.size () < 100) { // Yes, that is absolutely arbitrary
		std::string res;
		std::vector <char> list;
		if (this-> getType ().to <Type> ().getInners ()[0].to <Char> ().getSize () == 32)
		    list = validator::UtfVisitor::utf32_to_utf8 (this-> _value); // and ugly
		else list = this-> _value;
		
		for (auto & x : list) {
		    if (x != '\0')
			res += x;
		}
		
		return res;
	    } 
	    return Ymir::format ("_");
	}
	
    }
    
}
