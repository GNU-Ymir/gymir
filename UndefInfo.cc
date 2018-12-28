#include <ymir/semantic/types/_.hh>

namespace semantic {

    IUndefInfo::IUndefInfo () :
	IInfoType (true)
    {}
    
    IUndefInfo::IUndefInfo (bool willBeRef) :
	IInfoType (true)
    {
	this-> _willBeRef = willBeRef;
    }

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

    bool IUndefInfo::willBeRef () {
	return this-> _willBeRef;
    }
    
    std::string IUndefInfo::typeString () {
	if (this-> _willBeRef) {
	    return "ref (undef)";
	} 
	return "undef";
    }

    std::string IUndefInfo::innerTypeString () {
	if (this-> _willBeRef) {
	    return "ref (undef)";
	} 
	return "undef";
    }

    std::string IUndefInfo::innerSimpleTypeString () {
	if (this-> _willBeRef) {
	    return "ref (undef)";
	} 
	return "undef";
    }

    const char * IUndefInfo::getId () {
	return IUndefInfo::id ();
    }

    Ymir::Tree IUndefInfo::toGeneric () {
	return char_type_node;
    }
    
    
}
