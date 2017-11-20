#include <ymir/semantic/types/_.hh>

namespace semantic {

    const char * IPtrInfo::getId () {
	return IPtrInfo::id ();
    }

    std::string IPtrInfo::innerTypeString () {
	return "ptr!" + this-> _content-> innerTypeString ();
    }

    std::string IPtrInfo::simpleTypeString () {
	return "P" + this-> _content-> simpleTypeString ();
    }

    
}
