#include "utils/Mangler.hh"
#include <sstream>

namespace Mangler {

    std::string mangle_file (std::string & in) {
	std::stringstream ss;
	for (auto it : in) {
	    if (it == '/') ss << '.';
	    else ss << (char) it;
	}
	return ss.str ();
    }
    

}
