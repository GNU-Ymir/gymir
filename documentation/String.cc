#include <ymir/documentation/String.hh>
#include <ymir/utils/OutBuffer.hh>
#include <algorithm>

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
	    static std::vector <char> escape = {'a', 'b', 'f', 'n', 'r', 't', 'v', '\\', '\'', '\"', '"', '?'};
	    buf.write ("\"");
	    bool escaped = false;
	    for (auto & it : this-> _content) {
		if (escaped) {
		    auto pos = std::find (escape.begin (), escape.end (), it) - escape.begin ();
		    if (pos >= (int) escape.size ()) {
			buf.write ("\\\\");
		    } else {
			buf.write ("\\");
		    }
		    
		    if (it != '\0') buf.write (it);
		    escaped = false;
		} else {
		    if (it == '\"') buf.write ("\\\"");
		    else if (it == '\\') escaped = true;
		    else if (it != '\0') buf.write (it);
		}
		
	    }
	    if (escaped) buf.write ("\\\\");
	    buf.write ("\"");
	    return buf.str ();
	}
	
    }    

}
