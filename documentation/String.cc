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
	    Ymir::OutBuffer buf;
	    buf.write ("\"");
	    bool escaped = false;
	    for (auto & it : this-> _content) {
		if (it == '\"' && !escaped) buf.write ("\\\"");
		else buf.write (it);
		if (it == '\\') escaped = true;
		else escaped = false;
	    }
	    buf.write ("\"");
	    return buf.str ();
	}
	
    }    

}
