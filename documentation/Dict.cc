#include <ymir/documentation/Dict.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/utils/string.hh>

namespace documentation {

    namespace json {

	JsonDict::JsonDict () {}

	JsonDict::JsonDict (const std::map <std::string, JsonValue>  & val) :
	    _content (val)
	{}

	JsonValue JsonDict::init (const std::map <std::string, JsonValue>  & val) {
	    return JsonValue {new (NO_GC) JsonDict (val)};
	}
	
	JsonValue JsonDict::clone () const {
	    return JsonValue {new (NO_GC) JsonDict (*this)};
	}

	bool JsonDict::isOf (const IJsonValue * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    JsonDict thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return IJsonValue::isOf (type);	
	}

	std::string JsonDict::toString () const {
	    Ymir::OutBuffer buf;
	    int i = 0;
	    buf.writeln ("{");
	    for (auto & it : this-> _content) {
		if (i != 0) buf.writeln (", ");
		if (it.second.is <JsonDict> ()) {
		    buf.write (Ymir::format ("\t\"%\" : %", it.first, Ymir::entab (it.second.toString (), "\t")));
		} else {
		    buf.write (Ymir::format ("\t\"%\" : %", it.first, it.second.toString ()));
		}
		i += 1;
	    }
	    buf.write ("}");
	    return buf.str ();
	}
	
    }    

}
