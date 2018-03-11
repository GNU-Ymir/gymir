#include <ymir/semantic/types/MacroInfo.hh>

namespace semantic {

    using namespace syntax;
    
    IMacroInfo::IMacroInfo (Namespace space, std::string name, Macro elem) :
	IInfoType (true),
	_space (space),
	_name (name),
	_info (elem)
    {}

    bool IMacroInfo::isSame (InfoType) {
	return false;
    }

    std::string IMacroInfo::name () {
	return this-> _name;
    }

    Macro IMacroInfo::info () {
	return this-> _info;
    }

    Namespace IMacroInfo::space () {
	return this-> _space;
    }

    InfoType IMacroInfo::onClone () {
	return this;
    }

    MacroSolution IMacroInfo::resolve (MacroCall call) {
	return {false, {}};
    }

    std::string IMacroInfo::innerTypeString () {
	return this-> _name;
    }

    std::string IMacroInfo::typeString () {
	return innerTypeString ();
    }

    std::string IMacroInfo::innerSimpleTypeString () {
	return innerTypeString ();
    }

    const char* IMacroInfo::getId () {
	return this-> id ();
    }
    
}
