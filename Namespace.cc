#include "semantic/pack/Namespace.hh"
#include "utils/Mangler.hh"
#include <sstream>

namespace semantic {

    Namespace::Namespace (std::string name) {
	auto space = Mangler::mangle_file (name);
	auto index = space.find (".");
	while (index != std::string::npos) {
	    auto str = space.substr (0, index);
	    space = space.substr (index + 1, space.length ());
	    this-> names.push_back (str);
	    index = space.find (".");
	}
	this-> names.push_back (space);
    }
    
    Namespace::Namespace (const Namespace& space_, std::string name) {
	auto space = Mangler::mangle_file (name);
	auto index =  space.find (".");
	while (index != std::string::npos) {
	    auto str = space.substr (0, index);
	    space = space.substr (index + 1, space.length ());
	    this-> names.push_back (str);
	    index = space.find (".");
	}
	this-> names.push_back (space);

	this-> names.insert (this-> names.begin (),
			     space_.names.begin (),
			     space_.names.end ()
	);	
    }

    Namespace::Namespace (const std::vector <std::string> & names)
	: names (names)
    {}
    
    bool operator!= (const Namespace &fst, const Namespace &scd) {
	return !(fst == scd);
    }
    
    bool operator== (const Namespace &fst, const Namespace &scd) {
	return fst.toString () == scd.toString ();
    }

    const std::vector <std::string> & Namespace::innerMods () {
	return this-> names;
    }
    
    bool Namespace::isSubOf (const Namespace& other) const {
	if (this-> names.size () <= other. names.size ()) {
	    for (auto it = 0 ; it < (int) this-> names.size () ; it++) {
		if (other.names [it] != this-> names [it]) 
		    return false;		
	    }
	    return true;
	}
	return false;
    }

    bool Namespace::isAbsSubOf (const Namespace& other) const {
	if (this-> names.size () < other. names.size ()) {
	    for (auto it = 0 ; it < (int) this-> names.size () ; it++) {
		if (other.names [it] != this-> names [it])
		    return false;
	    }
	    return true;
	}
	return false;
    }

    std::string Namespace::toString () const {
	std::ostringstream ss;
	for (uint it = 0 ; it < this-> names.size () ; it ++) {
	    ss << this-> names [it];
	    if (it != this-> names.size () - 1) ss << ".";
	}
	return ss.str ();
    }
    
}
