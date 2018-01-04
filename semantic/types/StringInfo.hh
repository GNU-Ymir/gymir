#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    class IStringInfo : public IInfoType {
    private:

	static bool __initStringTypeNode__;

    public:

	IStringInfo (bool);

	bool isSame (InfoType) override;

	InfoType ConstVerif (InfoType) override;

	static InfoType create (Word tok, std::vector<syntax::Expression> tmps) {
	    if (tmps.size () != 0) {
		Ymir::Error::notATemplate (tok);
		return NULL;
	    } else {
		return new IStringInfo (false);
	    }
	}

	InfoType BinaryOp (Word tok, syntax::Expression right) override;
	
	InfoType BinaryOpRight (Word tok, syntax::Expression left) override;

	InfoType AccessOp (Word, syntax::ParamList, std::vector <InfoType> &) override;
	
	std::string innerTypeString () override;

	std::string simpleTypeString () override;

	InfoType clone () override;

	InfoType DotOp (syntax::Var) override;
	
	InfoType CompOp (InfoType) override;

	Ymir::Tree toGeneric () override;

	static Ymir::Tree toGenericStatic ();
	
	static const char* id () {
	    return "IStringInfo";
	}

	const char* getId () override;

	//TODO
    private:

	InfoType Ptr ();
	
	InfoType Length ();

	InfoType AffectRight (syntax::Expression);

	InfoType Affect (syntax::Expression);

	InfoType Access (syntax::Expression, InfoType&);
	
	InfoType Concat (syntax::Expression);
	
	InfoType ConcatAff (syntax::Expression);

    };

    typedef IStringInfo* StringInfo;
    
    
}
