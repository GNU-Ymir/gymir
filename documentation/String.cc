#include <ymir/documentation/String.hh>
#include <ymir/utils/OutBuffer.hh>

namespace documentation {

    namespace json {

	JsonString::JsonString () {}

	JsonString::JsonString (const std::string & val) :
	    _content (val)
	{}

	JsonValue JsonString::init (const std::string & val) {
	    return JsonValue {new (NO_GC) JsonString (val)};
	}

	JsonValue JsonString::init (ulong val) {
	    return JsonValue {new (NO_GC) JsonString (Ymir::format ("%", val))};
	}
	
	JsonValue JsonString::clone () const {
	    return JsonValue {new (NO_GC) JsonString (*this)};
	}

	bool JsonString::isOf (const IJsonValue * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    JsonString thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return IJsonValue::isOf (type);	
	}

	std::string JsonString::toString () const {
	    return Ymir::format ("\"%\"", this-> _content);
	}
	
    }    

}
