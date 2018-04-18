#include <ymir/semantic/types/_.hh>

namespace semantic {

    IUndefInfo::IUndefInfo () :
	IInfoType (true)
    {}

    bool IUndefInfo::isSame (InfoType) {	
	return false;
    }

    InfoType IUndefInfo::onClone () {
	return new (Z0)  IUndefInfo ();
    }

    InfoType IUndefInfo::DotOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	return NULL;
    }
    
    std::string IUndefInfo::typeString () {
	return "undef";
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

    Ymir::Tree IUndefInfo::toGeneric () {
	return char_type_node;
    }
    
    
}
