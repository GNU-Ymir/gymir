#pragma once

#include <ymir/semantic/types/InfoType.hh>
#include <vector>

namespace semantic {

    class ITupleInfo : public IInfoType {

	std::vector <InfoType> params;

    public:

	ITupleInfo (bool isConst);

	bool isSame (InfoType) override;

	InfoType ConstVerif (InfoType other) override;
	
	static InfoType create (Word token, std::vector <syntax::Expression> templates);

	InfoType BinaryOpRight (Word op, syntax::Expression left);

	InfoType BinaryOp (Word op, syntax::Expression right);
	
	InfoType CompOp (InfoType other);
		
	InfoType clone () override;

	InfoType DotExpOp (syntax::Expression right);

	InfoType DotOp (syntax::Var var);
	
	std::string innerTypeString () override;

	std::string simpleTypeString () override;
	
	Ymir::Tree toGeneric () override;
	
	void addParam (InfoType info);

	ulong nbParams ();

	std::vector <InfoType> &getParams ();

	void isConst (bool is) override;
	
	static const char* id () {
	    return "ITupleInfo";
	}

	const char* getId () override;

    private :

	InfoType Affect (Word tok, syntax::Expression right);

	InfoType AffectRight (Word tok, syntax::Expression left);

	InfoType SizeOf ();

	InfoType Empty ();
	
    };

    typedef ITupleInfo* TupleInfo;
    
}
