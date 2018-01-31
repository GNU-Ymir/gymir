#include <ymir/semantic/types/_.hh>

namespace semantic {

    IUndefInfo::IUndefInfo () :
	IInfoType (true)
    {}

    bool IUndefInfo::isSame (InfoType other) {	
	return false;
    }

    InfoType IUndefInfo::onClone () {
	return new (Z0)  IUndefInfo ();
    }

    InfoType IUndefInfo::DotOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "typeid") {
	    //TODO
	    return new (Z0)  IStringInfo (true);
	}
	return NULL;
    }

    std::string IUndefInfo::innerTypeString () {
	return "undef";
    }

    std::string IUndefInfo::innerSimpleTypeString () {
	return "undef";
    }

    const char * IUndefInfo::getId () {
	return IUndefInfo::id ();
    }
    
}
