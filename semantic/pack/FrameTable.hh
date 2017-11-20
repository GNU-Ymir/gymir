#pragma once

#include <vector>
#include "Frame.hh"
#include "FinalFrame.hh"

namespace semantic {

    class FrameTable {

	std::vector <Frame> pures;
	std::vector <FinalFrame> finals;
	std::vector <FinalFrame> finalTemplates;
	std::vector <FrameProto> protos;
	//std::vector <StructCstInfo> structs;
	//std::vector <ObjectCstInfo> objects;
	
    public:
	
	static FrameTable& instance () {
	    return __instance__;
	}
	
	
    private:

	static FrameTable __instance__;
	
    };
    
}
