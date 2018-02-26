#pragma once

#include <ymir/utils/Array.hh>

namespace Ymir {
    struct Tree;
}

namespace syntax {
    class IGlobal;
    typedef IGlobal* Global;
}

namespace semantic {

    class IFrame;
    typedef IFrame* Frame;

    class IFinalFrame;
    typedef IFinalFrame* FinalFrame;

    class IFrameProto;
    typedef IFrameProto* FrameProto;

    class IStructCstInfo;
    typedef IStructCstInfo* StructCstInfo;

    class IStructInfo;
    typedef IStructInfo* StructInfo;
    
    class ISymbol;
    typedef ISymbol* Symbol;
    
    class FrameTable {

	std::vector <Frame> _pures;
	std::vector <FinalFrame> _finals;
	std::vector <FinalFrame> _finalTemplates;
	std::vector <FrameProto> _protos;
	std::vector <StructCstInfo> _structs;
	std::map <std::string, std::vector <StructInfo> > _structIds;	
	std::vector <syntax::Global> _globals;
	std::vector <Symbol> _externals;
	//std::vector <ObjectCstInfo> objects;
	
	ulong _currentStructId = 0;
	
    public:
       	
	static FrameTable& instance () {
	    return __instance__;
	}

	void insert (Frame);

	void insert (StructCstInfo info);

	void insert (FinalFrame);

	void insert (syntax::Global);
	
	void insertExtern (Symbol);
	
	void insertTemplate (FinalFrame);

	void insert (FrameProto);

	FinalFrame existsFinal (std::string);

	bool existsProto (FrameProto&);

	//StructCstInfo existsStruct (std::string);

	void identify (StructInfo info);
	
	std::vector <StructCstInfo> & structs ();
	
	std::vector <Frame> & pures ();

	std::vector <FinalFrame>& finals ();

	std::vector <FinalFrame>& templates ();

	std::vector <syntax::Global> & globals ();

	std::vector <Symbol> & externals ();
	
	void addMain ();
	
	void purge ();
	
    private:

	Ymir::Tree allSelfCall ();

	Ymir::Tree allSelfDestCall ();
	
	static FrameTable __instance__;
	
    };
    
}
