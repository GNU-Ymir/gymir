#pragma once


#include <string>
#include <vector>
#include <map>

namespace semantic {
    
    class Creators {
    public:
	
	void * find (const std::string&);       	

	static Creators& instance () {
	    return __instance__;
	}
	
    private:

	Creators ();
	Creators (Creators&);
	Creators& operator=(Creators&);	
	
	std::map<std::string, void*> creators;
	
	static Creators __instance__;
	
    };

}
