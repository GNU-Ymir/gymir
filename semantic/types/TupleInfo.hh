#pragma once

#include <ymir/semantic/types/InfoType.hh>
#include <vector>

namespace semantic {

    class ITupleInfo : public IInfoType {

	std::vector <InfoType> params;
	bool _isFake;
	
    public:

	ITupleInfo (bool isConst);
	
	ITupleInfo (bool isConst, bool fake);

	bool isSame (InfoType) override;

	InfoType ConstVerif (InfoType other) override;

	bool passingConst (InfoType other) override;
	
	static InfoType create (Word token, const std::vector<syntax::Expression> & templates);

	InfoType BinaryOpRight (Word op, syntax::Expression left);

	InfoType BinaryOp (Word op, syntax::Expression right);
	
	InfoType CompOp (InfoType other);
		
	InfoType onClone () override;

	InfoType DotExpOp (syntax::Expression right) override;

	InfoType DColonOp (syntax::Var var) override;

	InfoType UnaryOp (Word) override;
	
	std::string innerTypeString () override;

	std::string innerSimpleTypeString () override;
	
	Ymir::Tree toGeneric () override;
	
	void addParam (InfoType info);

	ulong nbParams ();

	void setFake ();

	bool isFake ();
	
	InfoType asNoFake ();
	
	std::vector <InfoType> &getParams ();

	InfoType getTemplate (ulong) override;
	
	static const char* id () {
	    return "ITupleInfo";
	}

	const char* getId () override;

    private :

	InfoType Affect (Word tok, syntax::Expression right);

	InfoType AffectRight (Word tok, syntax::Expression left);

	InfoType SizeOf ();

	InfoType Empty ();

	InfoType Arity ();

	InfoType Init ();

	
    };

    typedef ITupleInfo* TupleInfo;
    
}
