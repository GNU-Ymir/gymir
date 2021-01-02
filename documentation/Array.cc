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

	std::string JsonArray::toString () const {
	    Ymir::OutBuffer buf;
	    int i = 0;
	    buf.write ("[");
	    for (auto & it : this-> _content) {
		if (it.is <JsonDict> ()) {
		    if (i != 0) buf.writeln (", ");
		    buf.write (Ymir::entab (it.toString (), "\t", i != 0));
		} else {
		    if (i != 0) buf.write (", ");
		    buf.write (it.toString ());
		}
		i += 1;
	    }
	    buf.write ("]");
	    return buf.str ();
	}
	
    }    

}
