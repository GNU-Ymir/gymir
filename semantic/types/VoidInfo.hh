#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    class IVoidInfo : public IInfoType {
    public:

	IVoidInfo ();

	bool isSame (InfoType) override;

	InfoType clone () override;

	InfoType DotOp (syntax::Var) override;

	std::string innerTypeString () override;

	std::string simpleTypeString () override;

	Ymir::Tree toGeneric () override;
	
	static const char* id () {
	    return "IVoidInfo";
	}

	const char* getId () override;

	
    };

    typedef IVoidInfo* VoidInfo;
    
}
