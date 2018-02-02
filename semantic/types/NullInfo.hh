#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    class INullInfo : public IInfoType {
    public:

	INullInfo ();

	bool isSame (InfoType) override;

	InfoType onClone () override;

	InfoType DotOp (syntax::Var) override;

	InfoType DColonOp (syntax::Var) override;

	InfoType CompOp (InfoType) override;

	std::string innerTypeString () override;

	std::string innerSimpleTypeString () override;

	static const char* id () {
	    return "INullInfo";
	}

	const char* getId () override;

	
    };

    typedef INullInfo* NullInfo;
    
}
