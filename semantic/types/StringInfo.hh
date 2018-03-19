#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    class IStringInfo : public IArrayInfo {
    private:

	static bool __initStringTypeNode__;
	bool _innerConst;
	bool _isImut;
	
    public:
	
	IStringInfo (bool, bool = true);
	
	static InfoType create (Word tok, const std::vector<syntax::Expression> & tmps) {
	    if (tmps.size () != 0) {
		Ymir::Error::notATemplate (tok);
		return NULL;
	    } else {
		return new (Z0) IStringInfo (false, true);
	    }
	}

	InfoType onClone () override;

	bool isConst () override;
	
	void isConst (bool isConst) override;
	
	InfoType BinaryOpRight (Word tok, syntax::Expression left) override;
	
    };

    typedef IStringInfo* StringInfo;
    
    
}
