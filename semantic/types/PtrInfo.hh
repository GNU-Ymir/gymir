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

	static InfoType create (Word tok, const std::vector<syntax::Expression> & tmps) {
	    if (tmps.size () != 1 || !tmps [0]-> is<syntax::IType> ()) {
		Ymir::Error::takeATypeAsTemplate (tok);
		return NULL;
	    } else {
		return new (Z0) IPtrInfo (false, tmps [0]-> info-> type);
	    }
	}

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

	InfoType& content ();

	InfoType getTemplate (ulong) override;
	
	static const char* id () {
	    return "IPtrInfo";
	}

	const char* getId () override;

	//TODO

    private:

	InfoType Affect (syntax::Expression right);

	InfoType AffectRight (syntax::Expression left);

	InfoType Plus (syntax::Expression right);

	InfoType Sub (syntax::Expression right);

	InfoType PlusRight (syntax::Expression left);

	InfoType SubRight (syntax::Expression left);

	InfoType Is (syntax::Expression right);

	InfoType NotIs (syntax::Expression right);

	InfoType Unref ();

	InfoType toPtr ();	
	
    };

    typedef IPtrInfo* PtrInfo;
    
    
}
