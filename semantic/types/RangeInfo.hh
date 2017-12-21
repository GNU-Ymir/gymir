#pragma once

#include <ymir/semantic/types/InfoType.hh>
#include <ymir/semantic/types/FixedInfo.hh>
#include <ymir/semantic/types/FloatInfo.hh>
#include <ymir/semantic/types/CharInfo.hh>
#include <ymir/ast/Type.hh>

namespace semantic {

    class IRangeInfo : public IInfoType {

	InfoType _content = NULL;

    public:

	IRangeInfo (bool);

	IRangeInfo (bool, InfoType);

	bool isSame (InfoType) override;

	static InfoType create (Word tok, std::vector<syntax::Expression> tmps) {
	    if (tmps.size () != 1 || !tmps [0]-> is<syntax::IType> ()) {
		Ymir::Error::takeATypeAsTemplate (tok);
		return NULL;
	    } else {
		auto type = tmps [0]-> info-> type;		
		if (type-> is <IFloatInfo> () || type-> is <IFixedInfo> () || type-> is <ICharInfo> ())
		    return new IRangeInfo (false, tmps [0]-> info-> type);
		else return NULL;
	    }
	}
	
	InfoType BinaryOp (Word token, syntax::Expression right) override;

	InfoType BinaryOpRight (Word token, syntax::Expression left) override;
		
	InfoType DotOp (syntax::Var var) override;
		
	InfoType CompOp (InfoType other) override;

	std::string innerTypeString () override;

	std::string simpleTypeString () override;

	static std::string simpleTypeStringStatic (std::string&);

	Ymir::Tree toGeneric () override;

	static Ymir::Tree toGenericStatic (std::string name, Ymir::Tree);
	
	InfoType clone () override;

	static const char* id () {
	    return "IRangeInfo";
	}

	const char* getId () override;

    private:

	InfoType Affect (syntax::Expression right);
	
	InfoType AffectRight (syntax::Expression left);

	InfoType In (syntax::Expression left);

	InfoType Fst ();

	InfoType Scd ();
	
    };

    typedef IRangeInfo* RangeInfo;
    
    
}
