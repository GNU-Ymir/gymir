#include <ymir/semantic/types/MacroInfo.hh>
#include <ymir/semantic/value/FixedValue.hh>
#include <ymir/ast/Macro.hh>
#include <ymir/semantic/tree/Tree.hh>

namespace semantic {
    using namespace Ymir;
    using namespace syntax;
    
    namespace MacroUtils {

	Tree InstNone (Word, InfoType, Expression, Expression) {
	    Ymir::Error::assert ("!!");
	    return NULL;
	}
	
	Tree InstGet (Word, InfoType, Expression left, Expression right) {
	    auto val = right-> token.getStr ();
	    auto macro = left-> info-> type ()-> to <IMacroAccessInfo> ();
	    for (auto it : macro-> getInfo ().elements) {
		if (it.first == val) {
		    return it.second-> toGeneric ();
		}
	    }
	    
	    Ymir::Error::assert ("!!!");
	    return Ymir::Tree ();
	}	

    }
       
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
	return MacroSolver::instance ().solve (this-> _info, call);
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
    
    IMacroRepeatInfo::IMacroRepeatInfo (MacroRepeat repeat) :
	IInfoType (true),
	info (repeat)
    {
    }

    bool IMacroRepeatInfo::isSame (InfoType) {
	return false;
    }

    ulong IMacroRepeatInfo::getLen () {
	return this-> info-> getSolution ().size ();
    }

    std::vector <Word> IMacroRepeatInfo::toTokens (bool& valid) {
	valid = true;
	return this-> info-> toTokens (valid);
    }
    
    MacroAccessInfo IMacroRepeatInfo::getValue (ulong i) {
	return new (Z0) IMacroAccessInfo (this-> info-> getSolution ()[i]);
    }
    
    InfoType IMacroRepeatInfo::onClone () {
	return this;
    }

    InfoType IMacroRepeatInfo::DotExpOp (syntax::Expression right) {
	if (!right-> info-> isImmutable ()) return NULL;
	if (right-> info-> value ()-> is<IFixedValue> ()) {
	    auto value = right-> info-> value ()-> to <IFixedValue> ()-> getValue ();
	    if (value >= (int) this-> getLen ()) return NULL;
	    else {
		auto ret = this-> getValue (value);
		ret-> binopFoo = &MacroUtils::InstNone;
		return ret;
	    }
	}
	return NULL;
    }
    
    std::string IMacroRepeatInfo::innerTypeString () {
	return "void";
    }

    std::string IMacroRepeatInfo::typeString () {
	return innerTypeString ();
    }

    std::string IMacroRepeatInfo::innerSimpleTypeString () {
	return innerTypeString ();
    }

    const char* IMacroRepeatInfo::getId () {
	return this-> id ();
    }

    IMacroAccessInfo::IMacroAccessInfo (MacroSolution soluce) :
	IInfoType (true),
	info (soluce)
    {}

    std::vector <Word> IMacroAccessInfo::toTokens (bool & valid) {
	std::vector <Word> tokens;
	valid = true;
	for (auto it : this-> info.elements) {
	     if (auto var = it.second-> to <IVar> ()) {
		tokens.push_back (var-> token);
	    } else if (auto elem = it.second-> to <IMacroElement> ()) {
		auto current = elem-> toTokens (valid);
		if (!valid) return {};
		tokens.insert (tokens.end (), current.begin (), current.end ());
	    }
	}
	return tokens;
    }
    
    bool IMacroAccessInfo::isSame (InfoType) {
	return false;
    }
    
    MacroSolution& IMacroAccessInfo::getInfo () {
	return this-> info;
    }
    
    InfoType IMacroAccessInfo::DotOp (syntax::Var var) {
	for (auto &it : this-> info.elements) {
	    if (it.first == var-> token.getStr ()) {
		if (!it.second-> info) {		
		    auto res = this-> cloneConst ()-> to<IMacroAccessInfo> ();
		    res-> info = {true, {{it.first, it.second}}, NULL};
		    return res;
		} else {
		    auto res = it.second-> info-> type ()-> cloneConst ();
		    res-> binopFoo = MacroUtils::InstGet;
		    return res;
		}
	    }
	}
	return NULL;
    }
    
    InfoType IMacroAccessInfo::onClone () {
	return this;
    }

    std::string IMacroAccessInfo::innerTypeString () {
	return "void";
    }

    std::string IMacroAccessInfo::typeString () {
	return innerTypeString ();
    }

    std::string IMacroAccessInfo::innerSimpleTypeString () {
	return innerTypeString ();
    }

    const char* IMacroAccessInfo::getId () {
	return this-> id ();
    }
    
}
