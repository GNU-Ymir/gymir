#pragma once

#include <ymir/documentation/Json.hh>

namespace documentation {

    namespace json {

	class JsonDict : public IJsonValue {
	private :
	    
	    std::map <std::string, JsonValue> _content;

	protected :

	    friend JsonValue;

	    JsonDict ();

	    JsonDict (const std::map <std::string, JsonValue> & content);


	public :

	    static JsonValue init (const std::map <std::string, JsonValue> & text);
	    
	    JsonValue clone () const override;

	    std::string toString () const override;
	    
	};	
	
    }
    
}
