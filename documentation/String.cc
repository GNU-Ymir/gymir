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

	std::string JsonString::toString () const {
	    return Ymir::format ("\"%\"", this-> _content);
	}
	
    }    

}
