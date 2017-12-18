#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    class IUndefInfo : public IInfoType {
    public:

	IUndefInfo ();

	bool isSame (InfoType) override;

	InfoType clone () override;

	InfoType DotOp (syntax::Var) override;

	std::string innerTypeString () override;

	std::string simpleTypeString () override;

	static const char* id () {
	    return "IUndefInfo";
	}

	const char* getId () override;

	
    };

    typedef IUndefInfo* UndefInfo;
    
}