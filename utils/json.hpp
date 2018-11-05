#pragma once

#include <vector>
#include <string>

namespace Ymir {

    enum class JSONKind {
	INT,
	STRING,
	ARRAY,
	OBJECT,
	UNDEF
    };
    
    struct json {
	JSONKind kind;
	std::vector <json> arr;
	std::map <std::string, json> obj;
	std::string str;
	int i;

	void push (json val) {
	    this-> kind = JSONKind::ARRAY;
	    this-> arr.push_back (val);	    
	}
	
	json& operator[] (std::string & val) {
	    this-> kind = JSONKind::OBJECT;
	    return this-> obj [val];
	}

	json& operator[] (const char* val) {
	    this-> kind = JSONKind::OBJECT;
	    return this-> obj [val];
	}

	static json undef () {
	    return json {JSONKind::UNDEF, {}, {}, "", 0};
	}

	std::string dump (int i = 0) {
	    Ymir::OutBuffer buf;
	    switch (this-> kind) {
	    case JSONKind::INT : buf.write (this-> i); break;
	    case JSONKind::STRING :
		{
		    size_t pos = 0;
		    while (true) {
			pos = this-> str.find ("\"", pos);
			if (pos != std::string::npos) {
			    this-> str.replace (pos, 1, "\\\"");
			    pos = pos + 2;
			} else break;
		    }
		
		    buf.write ("\"", this-> str, "\"");
		}
		break;
	    case JSONKind::ARRAY : {
		buf.write ("[\n");
		for (int z = 0 ; z < i ; z ++) buf.write (" ");
		int j = 0;
		for (auto & elem : this-> arr) {
		    if (j != 0) {
			buf.write (", \n");
			for (int z = 0 ; z < i ; z ++) buf.write (" ");
		    }
		    buf.write (elem.dump (i + 4));
		    j += 1;
		}
		buf.write ("\n");
		for (int z = 0 ; z < i - 4; z++) buf.write (" ");
		buf.write ("]");
	    } break;
	    default : {
		buf.write ("{\n");
		int j = 0;
		for (auto & pair : this-> obj) {
		    if (j != 0) buf.write (", \n");
		    for (int z = 0 ; z < i + 4; z++) buf.write (" ");
		    
		    buf.write ("\"", pair.first, "\"", " : ", pair.second.dump (i + 4));
		    j += 1;
		}
		buf.write ("\n");
		for (int z = 0 ; z < i - 4; z++) buf.write (" ");
		buf.write ("}");
	    } break;
	    }
	    return buf.str ();
	}

	int operator=(int i) {
	    this-> kind = JSONKind::INT;
	    this-> i = i;
	    return i;
	}
	
	std::string operator=(std::string val) {
	    this-> kind = JSONKind::STRING;
	    this-> str = val;
	    return val;
	}
   
	const char* operator=(const char * val) {
	    this-> kind = JSONKind::STRING;
	    this-> str = val;
	    return val;
	}

	std::vector <json>& operator=(std::vector<json>& val) {
	    this-> kind = JSONKind::ARRAY;
	    this-> arr = val;
	    return val;
	}
	
    };
    
}
