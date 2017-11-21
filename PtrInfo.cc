#include <ymir/semantic/types/_.hh>

namespace semantic {

    IPtrInfo::IPtrInfo (bool isConst) :
	IInfoType (isConst)
    {}
    
    IPtrInfo::IPtrInfo (bool isConst, InfoType type) :
	IInfoType (isConst),
	_content (type) 
    {}
    
    const char * IPtrInfo::getId () {
	return IPtrInfo::id ();
    }

    std::string IPtrInfo::innerTypeString () {
	return "ptr!" + this-> _content-> innerTypeString ();
    }

    std::string IPtrInfo::simpleTypeString () {
	return "P" + this-> _content-> simpleTypeString ();
    }

    InfoType IPtrInfo::clone () {
	return new IPtrInfo (this-> isConst (), this-> _content-> clone ());
    }

    InfoType IPtrInfo::ConstVerif (InfoType) {
	return NULL;
    }

    bool IPtrInfo::isSame (InfoType other) {
	if (auto ot = other-> to<IPtrInfo> ()) {
	    return ot-> _content-> isSame (this-> _content); 
	}
	return NULL;
    }
    
}
