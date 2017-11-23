#pragma once

#include <ymir/utils/Array.hh>
#include <string>

namespace semantic {

    struct Namespace {
    private:
	
	std::vector <std::string> names;

	Namespace () {}

    public:
	
	Namespace (std::string name);

	Namespace (Namespace space, std::string name);

	friend bool operator== (const Namespace &self, const Namespace &other);

	friend bool operator!= (const Namespace &self, const Namespace &other);

	bool isSubOf (Namespace other);

	bool isAbsSubOf (Namespace other);

	std::string toString ();
	
    };
    
}
