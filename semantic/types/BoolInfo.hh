#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    class IBoolInfo : IInfoType {
    public:
	
	IBoolInfo (bool isConst); 

	static InfoType create (Word w, std::vector <::syntax::Expression> tmps) {
	    if (tmps.size () != 0) {
		Ymir::Error::append (w, Ymir::Error::NotATemplate);
		return NULL;
	    } else return new IBoolInfo (false);
	}

	bool isSame (InfoType) override;

    };

    typedef IBoolInfo* BoolInfo;
        
}
