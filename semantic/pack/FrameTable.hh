#pragma once

#include <ymir/utils/Array.hh>

namespace semantic {

    class IFrame;
    typedef IFrame* Frame;

    class IFinalFrame;
    typedef IFinalFrame* FinalFrame;

    class IFrameProto;
    typedef IFrameProto* FrameProto;

    class IStructCstInfo;
    typedef IStructCstInfo* StructCstInfo;

    class ISymbol;
    typedef ISymbol* Symbol;
    
    class FrameTable {

	std::vector <Frame> _pures;
	std::vector <FinalFrame> _finals;
	std::vector <FinalFrame> _finalTemplates;
	std::vector <FrameProto> _protos;
	std::vector <StructCstInfo> _structs;
	std::vector <Symbol> _globals;
	std::vector <Symbol> _externals;
	//std::vector <ObjectCstInfo> objects;
	
    public:
       	
	static FrameTable& instance () {
	    return __instance__;
	}

	void insert (Frame);

	void insert (StructCstInfo info);

	void insert (FinalFrame);

	void insert (Symbol);
	
	void insertExtern (Symbol);
	
	void insertTemplate (FinalFrame);

	void insert (FrameProto);

	FinalFrame existsFinal (std::string);

	bool existsProto (FrameProto&);

	//StructCstInfo existsStruct (std::string);

	std::vector <StructCstInfo> & structs ();
	
	std::vector <Frame> & pures ();

	std::vector <FinalFrame>& finals ();

	std::vector <FinalFrame>& templates ();

	std::vector <Symbol> & globals ();

	std::vector <Symbol> & externals ();
	
	void addMain ();
	
	void purge ();
	
    private:

	static FrameTable __instance__;
	
    };
    
}
