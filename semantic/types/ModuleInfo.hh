#pragma once

#include <ymir/semantic/types/InfoType.hh>
#include <ymir/semantic/pack/Namespace.hh>
#include <ymir/semantic/pack/Module.hh>

namespace semantic {

    class IModuleInfo : public IInfoType {

	Module content;

    public:

	IModuleInfo (Module mod);

	bool isSame (InfoType) override;
	
	InfoType onClone () override;

	Module get ();

	std::string typeString () override;
	
	std::string innerTypeString () override;

	std::string innerSimpleTypeString () override;

	std::string simpleTypeString () override;
	
	static const char* id () {
	    return "IModuleInfo";
	}

	const char* getId () override;

	
    };

    typedef IModuleInfo* ModuleInfo;

}
