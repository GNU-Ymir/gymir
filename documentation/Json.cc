#include <ymir/documentation/Json.hh>

namespace documentation {

    namespace json {

	bool IJsonValue::isOf (const IJsonValue*) const {
	    return false;
	}

	JsonValue::JsonValue () :
	    RefProxy<IJsonValue, JsonValue> (nullptr)
	{}
	
	JsonValue::JsonValue (IJsonValue * val) :
	    RefProxy<IJsonValue, JsonValue> (val)
	{}

	
	JsonValue JsonValue::empty () {
	    return JsonValue {nullptr};
	}

	bool JsonValue::isEmpty () const {
	    return this-> _value == nullptr;
	}
	
	std::string JsonValue::toString () const {
	    if (this-> _value == nullptr) return "";
	    return this-> _value-> toString ();
	}
	
    }
    

}
