#pragma once

#include <ymir/semantic/types/InfoType.hh>
#include <ymir/semantic/pack/Namespace.hh>
#include <ymir/semantic/pack/Module.hh>
#include <ymir/ast/Module.hh>

namespace semantic {

    class IModuleInfo : public IInfoType {

	Module content;
	syntax::ModDecl decl;
	Namespace space;
	
    public:

	IModuleInfo (Module mod);

	IModuleInfo (Namespace space, syntax::ModDecl decl);
	
	bool isSame (InfoType) override;
	
	InfoType onClone () override;

	Module get ();

	InfoType TempOp (const std::vector<::syntax::Expression> &) override;
	
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
