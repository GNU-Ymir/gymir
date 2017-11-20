#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    class IPtrFuncInfo : public IInfoType {
    public:
	
	std::vector <InfoType> params;
	InfoType ret;
	ApplicationScore score;

	//TODO

	static const char* id () {
	    return "IPtrFuncInfo";
	}

	const char* getId () override;

	
	
    };

    typedef IPtrFuncInfo* PtrFuncInfo;
    
}
