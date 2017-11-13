#include "semantic/pack/Namespace.hh"
#include "utils/Mangler.hh"

namespace semantic {

    Namespace::Namespace (std::string name) {
	auto space = Mangler::mangle_file (name);
	auto index = space.find (".");
	while (index != -1) {
	    auto str = space.substr (0, index);
	    space = space.substr (index + 1, space.length ());
	    this-> names.push_back (str);
	    index = space.find (".");
	}
	this-> names.push_back (space);
    }
    
    Namespace::Namespace (Namespace space_, std::string name) {
	auto space = Mangler::mangle_file (name);
	auto index = space.find (".");
	while (index != -1) {
	    auto str = space.substr (0, index);
	    space = space.substr (index + 1, space.length ());
	    this-> names.push_back (str);
	    index = space.find (".");
	}
	this-> names.push_back (space);
	

	this-> names.insert (this-> names.begin (),
			     space_.names.begin (),
			     space_.names.end ());	
    }

    bool operator== (const Namespace &fst, const Namespace &scd) {
	if (fst.names.size () != scd.names.size ()) return false;
	for (int i = 0 ; i < fst.names.size () ; i++) {
	    if (fst.names [i] != scd.names [i]) return false;
	}
	return true;
    }

    bool Namespace::isSubOf (Namespace other) {
	if (this-> names.size () <= other. names.size ()) {
	    for (auto it = 0 ; it < (int) this-> names.size () ; it++) {
		if (other.names [it] != this-> names [it])
		    return false;
	    }
	    return true;
	}
	return false;
    }

    bool Namespace::isAbsSubOf (Namespace other) {
	if (this-> names.size () < other. names.size ()) {
	    for (auto it = 0 ; it < (int) this-> names.size () ; it++) {
		if (other.names [it] != this-> names [it])
		    return false;
	    }
	    return true;
	}
	return false;
    }
        
}
