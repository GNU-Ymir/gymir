#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    class ICharInfo : public IInfoType {
    public:
	
	ICharInfo (bool isConst); 

	static InfoType create (Word w, std::vector <::syntax::Expression> tmps) {
	    if (tmps.size () != 0) {
		Ymir::Error::notATemplate (w);
		return NULL;
	    } else return new ICharInfo (false);
	}

	bool isSame (InfoType) override;
	
	InfoType BinaryOp (Word, syntax::Expression) override;

	InfoType BinaryOpRight (Word, syntax::Expression) override;

	std::string innerTypeString () override;

	std::string innerSimpleTypeString () override;

	InfoType DotOp (syntax::Var) override;

	InfoType CastOp (InfoType) override;

	InfoType CompOp (InfoType) override;

	InfoType clone () override;

	Ymir::Tree toGeneric () override;
	
	static const char* id () {
	    return "ICharInfo";
	}

	const char* getId () override;

    private:

	InfoType Affect (syntax::Expression);

	InfoType AffectRight (syntax::Expression);

	InfoType opTest (Word, syntax::Expression);

	InfoType opTestRight (Word, syntax::Expression);

	InfoType opAff (Word op, syntax::Expression);

	InfoType opNorm (Word op, syntax::Expression);

	InfoType opNormRight (Word op, syntax::Expression);

	InfoType Init ();

	InfoType SizeOf ();

	InfoType StringOf ();
	
    };

    typedef ICharInfo* CharInfo;

}
