#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    class IBoolInfo : public IInfoType {
    public:
	
	IBoolInfo (bool isConst); 

	static InfoType create (Word w, const std::vector<::syntax::Expression> & tmps) {
	    if (tmps.size () != 0) {
		Ymir::Error::notATemplate (w);
		return NULL;
	    } else return new (Z0) IBoolInfo (false);
	}

	bool isSame (InfoType) override;
	
	InfoType BinaryOp (Word, syntax::Expression) override;

	InfoType BinaryOpRight (Word, syntax::Expression) override;

	InfoType UnaryOp (Word) override;

	std::string innerTypeString () override;

	std::string innerSimpleTypeString () override;

	InfoType DotOp (syntax::Var) override;

	InfoType DColonOp (syntax::Var) override;
		
	InfoType CastOp (InfoType) override;

	InfoType CompOp (InfoType) override;

	InfoType onClone () override;
		
	static const char* id () {
	    return "IBoolInfo";
	}

	const char* getId () override;

	Ymir::Tree toGeneric () override;
	
    private:

	InfoType Ptr ();

	InfoType Affect (syntax::Expression );

	InfoType AffectRight (syntax::Expression );

	InfoType opNorm (Word, syntax::Expression);

	InfoType opReaff (Word, syntax::Expression);
	
	InfoType Init ();

	InfoType SizeOf ();
	
    };

    typedef IBoolInfo* BoolInfo;
        
}
