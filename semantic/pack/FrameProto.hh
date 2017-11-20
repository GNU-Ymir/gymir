#pragma once

#include <gc/gc_cpp.h>


namespace semantic {

    class IInfoType;
    typedef IInfoType* InfoType;
    
    class IFrameProto : public gc {
    public:
	
	std::string name;
	InfoType type;

    };

    typedef IFrameProto* FrameProto;
    
}
