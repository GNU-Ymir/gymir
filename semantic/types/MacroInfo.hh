#pragma once

#include <ymir/semantic/types/InfoType.hh>
#include <ymir/semantic/pack/Frame.hh>
#include <ymir/semantic/pack/Namespace.hh>
#include <ymir/semantic/pack/MacroSolver.hh>
#include <vector>
#include <map>
#include <ymir/utils/Array.hh>

namespace syntax {
    class IMacro;
    typedef IMacro* Macro;

    class IMacroCall;
    typedef IMacroCall* MacroCall;

    class IMacroRepeat;
    typedef IMacroRepeat* MacroRepeat;
}

namespace semantic {

    class IMacroInfo : public IInfoType {
	semantic::Namespace _space;
	std::string _name;
	syntax::Macro _info;
	
    public :

	IMacroInfo (Namespace, std::string, syntax::Macro elem);

	bool isSame (InfoType) override;

	std::string name ();

	syntax::Macro info ();
	    
	Namespace space ();

	InfoType onClone () override;

	MacroSolution resolve (syntax::MacroCall call);

	std::string innerTypeString () override;

	std::string typeString () override;

	std::string innerSimpleTypeString () override;

	static const char* id () {
	    return "IMacroInfo";
	}

	const char* getId () override;
       	
    };

    class IMacroAccessInfo : public IInfoType {

	MacroSolution info;
	
    public :

	IMacroAccessInfo (MacroSolution elem);

	bool isSame (InfoType) override;

	InfoType onClone () override;
	
	InfoType DotOp (syntax::Var);
	
	MacroSolution& getInfo ();

	std::string innerTypeString () override;

	std::string typeString () override;

	std::string innerSimpleTypeString () override;

	static const char* id () {
	    return "IMacroAccessInfo";
	}

	const char* getId () override;
    };


    class IMacroRepeatInfo : public IInfoType {
	syntax::MacroRepeat info;
	
    public :

	IMacroRepeatInfo (syntax::MacroRepeat elem);
	
	bool isSame (InfoType) override;

	ulong getLen ();

	IMacroAccessInfo* getValue (ulong i);

	InfoType onClone () override;
	
	InfoType DotExpOp (syntax::Expression) override;

	std::string innerTypeString () override;

	std::string typeString () override;

	std::string innerSimpleTypeString () override;

	static const char* id () {
	    return "IMacroRepeatInfo";
	}

	const char* getId () override;
       	
    };

    
    typedef IMacroInfo* MacroInfo;
    typedef IMacroRepeatInfo* MacroRepeatInfo;
    typedef IMacroAccessInfo* MacroAccessInfo;
}
