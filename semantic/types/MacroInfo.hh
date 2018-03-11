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

    typedef IMacroInfo* MacroInfo;

}
