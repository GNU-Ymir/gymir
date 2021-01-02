#pragma once

#include <ymir/documentation/Json.hh>

namespace documentation {

    namespace json {


	class JsonString : public IJsonValue {
	private :

	    std::string _content;

	protected :

	    friend JsonValue;

	    JsonString ();

	    JsonString (const std::string & content);


	public :

	    static JsonValue init (const std::string & text);
	    
	    static JsonValue init (ulong text);

	    JsonValue clone () const override;

	    std::string toString () const override;
	    
	};
		
    }    

}
