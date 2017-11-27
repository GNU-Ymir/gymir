#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    class IStringInfo : public IInfoType {
    public:

	IStringInfo (bool);

	bool isSame (InfoType) override;

	InfoType ConstVerif (InfoType) override;

	static InfoType create (Word tok, std::vector<syntax::Expression> tmps) {
	    if (tmps.size () != 0) {
		Ymir::Error::notATemplate (tok);
		return NULL;
	    } else {
		return new IStringInfo (false);
	    }
	}

	std::string innerTypeString () override;

	std::string simpleTypeString () override;

	InfoType clone () override;
	
	static const char* id () {
	    return "IStringInfo";
	}

	const char* getId () override;

	//TODO
	
    };

    typedef IStringInfo* StringInfo;
    
    
}
