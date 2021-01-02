#pragma once

#include <ymir/documentation/Json.hh>

namespace documentation {

    namespace json {

	class JsonArray : public IJsonValue {
	private :
	    
	    std::vector <JsonValue> _content;

	protected :

	    friend JsonValue;

	    JsonArray ();

	    JsonArray (const std::vector <JsonValue> & content);


	public :

	    static JsonValue init (const std::vector <JsonValue> & text);
	    
	    JsonValue clone () const override;

	    std::string toString () const override;
	    
	};	
	
    }
    
}
