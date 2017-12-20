#include <ymir/semantic/types/_.hh>

namespace semantic {

    IUndefInfo::IUndefInfo () :
	IInfoType (true)
    {}

    bool IUndefInfo::isSame (InfoType) {
	return false;
    }

    InfoType IUndefInfo::clone () {
	return new IUndefInfo ();
    }

    InfoType IUndefInfo::DotOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "typeid") {
	    //TODO
	    return new IStringInfo (true);
	}
	return NULL;
    }

    std::string IUndefInfo::innerTypeString () {
	return "undef";
    }

    std::string IUndefInfo::simpleTypeString () {
	return "undef";
    }

    const char * IUndefInfo::getId () {
	return IUndefInfo::id ();
    }
    
}
