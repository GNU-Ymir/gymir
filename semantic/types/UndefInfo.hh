#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    class IUndefInfo : public IInfoType {
    public:

	IUndefInfo ();

	bool isSame (InfoType) override;

	InfoType onClone () override;

	InfoType DotOp (syntax::Var) override;

	std::string typeString () override;
	
	std::string innerTypeString () override;

	std::string innerSimpleTypeString () override;

	static const char* id () {
	    return "IUndefInfo";
	}

	Ymir::Tree toGeneric () override;
	
	const char* getId () override;

	
    };

    typedef IUndefInfo* UndefInfo;
    
}
