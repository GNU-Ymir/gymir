#include <ymir/semantic/types/_.hh>

namespace semantic {

    INullInfo::INullInfo () :
	IInfoType (true)
    {}

    bool INullInfo::isSame (InfoType other) {
	return other-> is<INullInfo> ();
    }

    InfoType INullInfo::clone () {
	return new INullInfo ();
    }

    InfoType INullInfo::DotOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "typeid") {
	    //TODO
	    return new IStringInfo (true);
	}
	return NULL;
    }

    InfoType INullInfo::CompOp (InfoType other) {
	// if (other-> is<IStructInfo> ()) {
	//     //TODO
	//     return other-> clone ();
	// } else
	if (other-> is<IArrayInfo> ()) {
	    return other-> clone ();
	} else if (other-> is<IStringInfo> ()) {
	    return other-> clone ();
	} else if (other-> is<IPtrInfo> ()) {
	    return other-> clone ();
	} else if (other-> is <IPtrFuncInfo> ()) {
	    return other-> clone ();
	} else if (other-> is<INullInfo> ()) {
	    return other-> clone ();
	}
	return NULL;
    }

    std::string INullInfo::innerTypeString () {
	return "null";
    }

    std::string INullInfo::simpleTypeString () {
	return "n";
    }

    const char * INullInfo::getId () {
	return INullInfo::id ();
    }
    
}
