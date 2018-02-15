#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/utils/FixedUtils.hh>

namespace semantic {

    INullInfo::INullInfo () :
	IInfoType (true)
    {}

    bool INullInfo::isSame (InfoType other) {
	return other-> is<INullInfo> ();
    }

    InfoType INullInfo::onClone () {
	return new (Z0)  INullInfo ();
    }

    InfoType INullInfo::DotOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	return NULL;
    }

    InfoType INullInfo::DColonOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	return NULL;
    }

    
    InfoType INullInfo::CompOp (InfoType other) {
	if (other-> is<IPtrInfo> ()) {
	    auto ret = other-> clone ();
	    ret-> binopFoo = &FixedUtils::InstCast;
	    return ret;
	} else if (other-> is <IPtrFuncInfo> ()) {
	    auto ret = other-> clone ();
	    ret-> binopFoo = &FixedUtils::InstCast;
	    return ret;
	}
	return NULL;
    }

    std::string INullInfo::innerTypeString () {
	return "null";
    }

    std::string INullInfo::innerSimpleTypeString () {
	return "n";
    }

    const char * INullInfo::getId () {
	return INullInfo::id ();
    }
    
}
