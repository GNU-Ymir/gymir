#include <ymir/semantic/types/_.hh>

namespace semantic {

    IVoidInfo::IVoidInfo () :
	IInfoType (true)
    {}

    bool IVoidInfo::isSame (InfoType other) {
	return false;
    }

    InfoType IVoidInfo::clone () {
	return new IVoidInfo ();
    }

    InfoType IVoidInfo::DotOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "typeid") {
	    //TODO
	    return new IStringInfo (true);
	}
    }

    std::string IVoidInfo::innerTypeString () {
	return "undef";
    }

    std::string IVoidInfo::simpleTypeString () {
	return "undef";
    }

    const char * IVoidInfo::getId () {
	return IVoidInfo::id ();
    }
    
}
