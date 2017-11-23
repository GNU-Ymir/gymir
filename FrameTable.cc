#include "semantic/pack/FrameTable.hh"

namespace semantic {
    
    FrameTable FrameTable::__instance__;

    void FrameTable::insert (Frame fr) {
	this-> _pures.push_back (fr);
    }

    void FrameTable::insert (FinalFrame fr) {
	this-> _finals.push_back (fr);
    }
	
    void FrameTable::insertTemplate (FinalFrame fr) {
	this-> _finalTemplates.push_back (fr);
    }

    void FrameTable::insert (FrameProto fr) {
	this-> _protos.push_back (fr);
    }

    FinalFrame FrameTable::existsFinal (std::string name) {
	for (auto it : this-> _finals) {
	    if (it-> name () == name) return it;
	}

	for (auto it : this-> _finalTemplates) {
	    if (it-> name () == name) return it;
	}
	
	return NULL;
    }

    bool FrameTable::existsProto (FrameProto& proto) {
	for (auto it : this-> _protos) {
	    if (it-> equals (proto)) {
		proto-> type () = it-> type ();
		return true;
	    }
	}
	return false;
    }

    //StructCstInfo existsStruct (std::string);

    std::vector <Frame> & FrameTable::pures () {
	return this-> _pures;
    }

    std::vector <FinalFrame>& FrameTable::finals () {
	return this-> _finals;
    }

    std::vector <FinalFrame>& FrameTable::templates () {
	return this-> _finalTemplates;
    }

    void FrameTable::purge () {
	this-> _finals.clear ();
	this-> _finalTemplates.clear ();
	this-> _pures.clear ();
	this-> _protos.clear ();
    }
    
}
