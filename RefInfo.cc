#include <ymir/semantic/types/_.hh>

namespace semantic {

    IRefInfo::IRefInfo (bool isConst) :
	IInfoType (isConst),
	_content (NULL)
    {}

    IRefInfo::IRefInfo (bool isConst, InfoType content) :
	IInfoType (isConst),
	_content (content)
    {}

    bool IRefInfo::isSame (InfoType type) {
	if (auto ot = type-> to<IRefInfo> ()) {
	    return ot-> _content-> isSame (this-> _content);
	}
	return false;
    }

    InfoType IRefInfo::ConstVerif (InfoType) {
	return NULL;
    }

    std::string IRefInfo::innerTypeString () {
	return std::string ("ref(") + this-> _content-> innerTypeString () + ")";
    }

    std::string IRefInfo::simpleTypeString () {
	return std::string ("R") + this-> _content-> simpleTypeString ();
    }

    InfoType IRefInfo::clone () {
	return new IRefInfo (this-> isConst (), this-> _content-> clone ());
    }

    const char* IRefInfo::getId () {
	return IRefInfo::id ();
    }

    InfoType IRefInfo::content () {
	return this-> _content;
    }
    
    
}
