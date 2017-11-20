#pragma once

#include <ymir/semantic/types/InfoType.hh>
#include <ymir/ast/Type.hh>

namespace semantic {


    class IRangeInfo : public IInfoType {

	InfoType _content = NULL;

    public:

	IRangeInfo (bool);

	IRangeInfo (bool, InfoType);

	bool isSame (InfoType) override;

	InfoType ConstVerif (InfoType) override;

	static InfoType create (Word tok, std::vector<syntax::Expression> tmps) {
	    if (tmps.size () != 1 || !tmps [0]-> is<syntax::IType> ()) {
		Ymir::Error::append (tok, Ymir::TakeAType);
		return NULL;
	    } else {
		return new IRangeInfo (false, tmps [0]-> info-> type);
	    }
	}
	
	std::string innerTypeString () override;

	std::string simpleTypeString () override;

	InfoType clone () override;

	static const char* id () {
	    return "IRangeInfo";
	}

	const char* getId () override;

	//TODO
	
    };

    typedef IRangeInfo* RangeInfo;
    
    
}
