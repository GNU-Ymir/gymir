#include <ymir/semantic/pack/Table.hh>
#include <ymir/semantic/types/ModuleInfo.hh>
#include <ymir/ast/Var.hh>

namespace semantic {
    
    IModuleInfo::IModuleInfo (Module content) :
	IInfoType (true),
	content (content)
    {}

    bool IModuleInfo::isSame (InfoType other) {
	return this == other;
    }

    InfoType IModuleInfo::onClone () {
	return this;
    }
    
    Module IModuleInfo::get () {
	return this-> content;
    }
    
    std::string IModuleInfo::typeString () {
	return "Module::" + this-> content-> space ().toString ();
    }

    std::string IModuleInfo::innerTypeString () {
	return "Module::" + this-> content-> space ().toString ();
    }

    std::string IModuleInfo::innerSimpleTypeString () {
	Ymir::Error::assert ("!!!");
	return "";
    }

    std::string IModuleInfo::simpleTypeString () {
	Ymir::Error::assert ("!!!");
	return "";
    }
    
    const char * IModuleInfo::getId () {
	return IModuleInfo::id ();
    }        

}
