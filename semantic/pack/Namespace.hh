#pragma once

#include <vector>
#include <string>

namespace semantic {

    struct Namespace {
    private:
	
	std::vector <std::string> names;

	Namespace () {}

    public:
	
	Namespace (std::string name);

	Namespace (Namespace space, std::string name);

	friend bool operator== (Namespace &self, Namespace &other);

	bool isSubOf (Namespace other);

	bool isAbsSubOf (Namespace other);

	Namespace addSuffix (std::string suff);

	std::string directory ();

	std::string asFile (std::string ext);

	std::string toString ();
    };
    
}
