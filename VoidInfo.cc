#include <ymir/semantic/types/_.hh>

namespace semantic {

    IVoidInfo::IVoidInfo () :
	IInfoType (false)
    {}

    bool IVoidInfo::isSame (InfoType other) {
	return other-> is <IVoidInfo> ();
    }

    InfoType IVoidInfo::onClone () {
	return new (Z0)  IVoidInfo ();
    }

    InfoType IVoidInfo::DotOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "typeid") {
	    return StringOf ();
	}
	return NULL;
    }

    Ymir::Tree IVoidInfo::toGeneric () {
	return void_type_node;
    }    
    
    std::string IVoidInfo::typeString () {
	return "void";
    }

    std::string IVoidInfo::innerTypeString () {
	return "void";
    }

    std::string IVoidInfo::innerSimpleTypeString () {
	return "v";
    }

    const char * IVoidInfo::getId () {
	return IVoidInfo::id ();
    }
    
}
