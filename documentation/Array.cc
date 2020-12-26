#include <ymir/documentation/Array.hh>
#include <ymir/documentation/Dict.hh>
#include <ymir/utils/OutBuffer.hh>
#include <ymir/utils/string.hh>

namespace documentation {

    namespace json {

	JsonArray::JsonArray () {}

	JsonArray::JsonArray (const std::vector <JsonValue>  & val) :
	    _content (val)
	{}

	JsonValue JsonArray::init (const std::vector <JsonValue>  & val) {
	    return JsonValue {new (NO_GC) JsonArray (val)};
	}
	
	JsonValue JsonArray::clone () const {
	    return JsonValue {new (NO_GC) JsonArray (*this)};
	}

	bool JsonArray::isOf (const IJsonValue * type) const {
	    auto vtable = reinterpret_cast <const void* const *> (type) [0];
	    JsonArray thisValue; // That's why we cannot implement it for all class
	    if (reinterpret_cast <const void* const *> (&thisValue) [0] == vtable) return true;
	    return IJsonValue::isOf (type);	
	}

	std::string JsonArray::toString () const {
	    Ymir::OutBuffer buf;
	    int i = 0;
	    buf.write ("[");
	    for (auto & it : this-> _content) {
		if (i != 0) buf.write (", ");
		if (it.is <JsonDict> ()) {
		    buf.write (Ymir::entab (it.toString (), "\t"));
		} else {
		    buf.write (it.toString ());
		}
		i += 1;
	    }
	    buf.write ("]");
	    return buf.str ();
	}
	
    }    

}
