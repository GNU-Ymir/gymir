#pragma once

#include <ymir/utils/Array.hh>

namespace semantic {

    class IFrame;
    typedef IFrame* Frame;

    class IFinalFrame;
    typedef IFinalFrame* FinalFrame;

    class IFrameProto;
    typedef IFrameProto* FrameProto;

    class FrameTable {

	std::vector <Frame> _pures;
	std::vector <FinalFrame> _finals;
	std::vector <FinalFrame> _finalTemplates;
	std::vector <FrameProto> _protos;
	//std::vector <StructCstInfo> structs;
	//std::vector <ObjectCstInfo> objects;
	
    public:
       	
	static FrameTable& instance () {
	    return __instance__;
	}

	void insert (Frame);

	void insert (FinalFrame);
	
	void insertTemplate (FinalFrame);

	void insert (FrameProto);

	FinalFrame existsFinal (std::string);

	bool existsProto (FrameProto&);

	//StructCstInfo existsStruct (std::string);

	std::vector <Frame> & pures ();

	std::vector <FinalFrame>& finals ();

	std::vector <FinalFrame>& templates ();
	
	void purge ();
	
    private:

	static FrameTable __instance__;
	
    };
    
}
