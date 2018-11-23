#pragma once

#include <ymir/semantic/types/InfoType.hh>
#include <ymir/ast/Type.hh>

namespace semantic {

       
    class IPtrInfo : public IInfoType {

	InfoType _content = NULL;

    public:

	IPtrInfo (bool);

	IPtrInfo (bool, InfoType);

	bool isSame (InfoType) override;

	InfoType ConstVerif (InfoType) override;

	bool passingConst (InfoType) override;

	bool needKeepConst () override;
	
	static InfoType create (Word tok, const std::vector<syntax::Expression> & tmps);
	
	InfoType BinaryOp (Word token, syntax::Expression right) override;
	
	InfoType BinaryOpRight (Word token, syntax::Expression right) override;
	
	InfoType UnaryOp (Word op) override;

	InfoType DotOp (syntax::Var var) override;

	InfoType DColonOp (syntax::Var) override;

	InfoType CastOp (InfoType other) override;

	InfoType CompOp (InfoType other) override;
	
	std::string innerTypeString () override;
	
	std::string innerSimpleTypeString () override;

	InfoType onClone () override;
	
	Ymir::Tree toGeneric () override;

	Ymir::Tree genericConstructor () override;

	Ymir::Tree genericTypeInfo () override;
	
	InfoType& content ();

	InfoType getTemplate (ulong) override;
	
	static const char* id () {
	    return "IPtrInfo";
	}

	const char* getId () override;

	bool isConst () override;

	void isConst (bool isConst) override;

    private:

	InfoType Affect (syntax::Expression right);

	InfoType AffectRight (syntax::Expression left);

	InfoType Plus (syntax::Expression right);

	InfoType Sub (syntax::Expression right);

	InfoType PlusRight (syntax::Expression left);

	InfoType SubRight (syntax::Expression left);

	InfoType Is (syntax::Expression right);

	InfoType NotIs (syntax::Expression right);

	InfoType Unref (Word&);

	InfoType toPtr ();

	InfoType addUnref (InfoType elem);
	
    };

    typedef IPtrInfo* PtrInfo;
    
    
}
