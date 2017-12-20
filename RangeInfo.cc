#include <ymir/semantic/types/_.hh>
#include <ymir/errors/Error.hh>

namespace semantic {

    IRangeInfo::IRangeInfo (bool isConst) :
	IInfoType (isConst),
	_content (new IVoidInfo ())
    {}
    
    IRangeInfo::IRangeInfo (bool isConst, InfoType content) :
	IInfoType (isConst),
	_content (content)
    {}

    bool IRangeInfo::isSame (InfoType other) {
	if (auto ot = other-> to<IRangeInfo> ()) {
	    return ot-> _content-> isSame (this-> _content);
	}
	return false;
    }
    
    InfoType IRangeInfo::ConstVerif (InfoType) {
	Ymir::Error::assert ("TODO");
	return NULL;
    }
    
    InfoType IRangeInfo::clone () {
	return new IRangeInfo (this-> isConst (), this-> _content-> clone ());
    }
    
    const char * IRangeInfo::getId () {
	return IRangeInfo::id ();
    }
    
    std::string IRangeInfo::innerTypeString () {
	return "range!" + this-> _content-> innerTypeString ();
    }

    std::string IRangeInfo::simpleTypeString () {
	return "R_" + this-> _content-> simpleTypeString ();
    }

    
}
