#include <ymir/semantic/types/_.hh>

namespace semantic {

    const char * IRangeInfo::getId () {
	return IRangeInfo::id ();
    }
    
    std::string IRangeInfo::innerTypeString () {
	return "range!" + this-> _content-> innerTypeString ();
    }

    std::string IRangeInfo::simpleTypeString () {
	return "R_" + this-> _content-> simpleTypeString ();
    }

    
}
