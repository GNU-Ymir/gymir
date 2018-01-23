#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    class IVoidInfo : public IInfoType {
    public:

	IVoidInfo ();

	bool isSame (InfoType) override;

	static InfoType create (Word tok, const std::vector<syntax::Expression> & tmps) {
	    if (tmps.size () != 0) 
		Ymir::Error::notATemplate (tok);
	    return new (GC) IVoidInfo ();
	}
	
	InfoType clone () override;

	InfoType DotOp (syntax::Var) override;

	std::string typeString () override;
	
	std::string innerTypeString () override;

	std::string innerSimpleTypeString () override;

	Ymir::Tree toGeneric () override;
	
	static const char* id () {
	    return "IVoidInfo";
	}

	const char* getId () override;

	
    };

    typedef IVoidInfo* VoidInfo;
    
}
