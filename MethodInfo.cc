#include <ymir/semantic/object/MethodInfo.hh>

namespace semantic {
    
    IMethodInfo::IMethodInfo (AggregateInfo info, std::string name, const std::vector <Frame> & frames, const std::vector <int> & index) :
	IInfoType (true),
	_info (info),
	_name (name),
	_frames (frames),
	_index (index)
    {}

    bool IMethodInfo::isSame (InfoType) {
	return false;
    }

    InfoType IMethodInfo::onClone () {
	return new (Z0) IMethodInfo (this-> _info, this-> _name, this-> _frames, this-> _index);
    }

    std::string IMethodInfo::innerTypeString () {
	return "method <" + this-> _info-> getName () + "." + this-> _name + ">";
    }

    std::string IMethodInfo::innerSimpleTypeString () {
	Ymir::Error::assert ("TODO");
	return "";
    }

    const char* IMethodInfo::getId () {
	return IMethodInfo::id ();
    }            
    
}
