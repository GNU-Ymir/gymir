#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace syntax {

    class ITypedVar;
    class ITrait;
    class IExpression;
    struct TraitProto;


    
    typedef ITypedVar* TypedVar;
    typedef ITrait* Trait;
    typedef IExpression* Expression;
    
}

namespace semantic {

    class ITraitInfo : public IInfoType {
	
	Namespace _space;	
	syntax::Trait _info;
	
    public :

	ITraitInfo (Namespace space, syntax::Trait info);

	bool isSame (InfoType) override;

	bool validate (InfoType);
	
	InfoType onClone () override;

	std::string innerTypeString () override;

	std::string innerSimpleTypeString () override;

	InfoType TempOp (const std::vector <::syntax::Expression> &) override;
	
	static const char* id () {
	    return "ITraitInfo";
	}

	const char* getId () override;

    };

    typedef ITraitInfo* TraitInfo;
    

}
