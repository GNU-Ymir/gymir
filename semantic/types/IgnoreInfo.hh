#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    class IIgnoreInfo : public IInfoType {
    public:

	IIgnoreInfo ();

	bool isSame (InfoType) override;
	
	InfoType clone () override;

	InfoType BinaryOp (Word op, syntax::Expression left) override;
	
	std::string typeString () override;
	
	std::string innerTypeString () override;
	
	std::string innerSimpleTypeString () override;
	
	Ymir::Tree toGeneric () override;       

	static const char* id () {
	    return "IIgnoreInfo";
	}
	
	const char* getId () override;
	
    };

    typedef IIgnoreInfo* IgnoreInfo;
    
}
