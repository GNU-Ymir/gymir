#include <ymir/semantic/types/_.hh>

namespace semantic {


    IStringInfo::IStringInfo (bool isConst) :
	IInfoType (isConst)
    {}

    bool IStringInfo::isSame (InfoType other) {
	return other-> is<IStringInfo> ();
    }

    std::string IStringInfo::innerTypeString () {
	return "string";
    }

    std::string IStringInfo::simpleTypeString () {
	if (this-> isConst ()) return "cs";
	else return "s";
    }

    InfoType IStringInfo::clone () {
	return new IStringInfo (this-> isConst ());
    }

    const char* IStringInfo::getId () {
	return IStringInfo::id ();
    }

    InfoType IStringInfo::ConstVerif (InfoType) {
	return NULL;
    }

}
