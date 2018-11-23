#pragma once

#include <ymir/semantic/types/InfoType.hh>
#include <ymir/ast/Type.hh>

namespace semantic {

    class IRefInfo : public IInfoType {

	InfoType _content = NULL;

    public:

	IRefInfo (bool);

	IRefInfo (bool, InfoType);

	bool isSame (InfoType) override;

	InfoType ConstVerif (InfoType) override;

	bool passingConst (InfoType) override;
	
	static InfoType create (Word tok, const std::vector<syntax::Expression> & tmps) {
	    if (tmps.size () != 1 || !tmps [0]-> is<syntax::IType> ()) {
		Ymir::Error::takeATypeAsTemplate (tok);
		return NULL;
	    } else {
		return new (Z0) IRefInfo (false, tmps [0]-> info-> type ());
	    }
	}

	InfoType BinaryOp (Word token, syntax::Expression right) override;

	InfoType BinaryOpRight (Word token, syntax::Expression left) override;

	InfoType AccessOp (Word token, syntax::ParamList params, std::vector <InfoType> &) override;

	InfoType DotOp (syntax::Var var) override;

	InfoType DotExpOp (syntax::Expression elem) override;

	InfoType DColonOp (syntax::Var var) override;

	InfoType UnaryOp (Word op) override;

	InfoType CastOp (InfoType other) override;

	InfoType CompOp (InfoType other) override;

	InfoType ApplyOp (const std::vector<syntax::Var> & vars) override;
	
	ApplicationScore CallType (Word op, syntax::ParamList params) override;
	
	ApplicationScore CallOp (Word op, syntax::ParamList params) override;

	bool isConst () override;

	void isConst (bool set) override;
	
	std::string innerTypeString () override;

	std::string innerSimpleTypeString () override;
	
	InfoType onClone () override;
	
	static const char* id () {
	    return "IRefInfo";
	}

	Ymir::Tree toGeneric () override;
	
	Ymir::Tree genericTypeInfo () override;

	const char* getId () override;

	InfoType content ();

	Symbol& symbol () override;
	
	bool isLvalue () override;
	
    private:

	InfoType Unref ();	

	InfoType addUnref (InfoType elem);

	InfoType addUnrefRight (InfoType elem);

	InfoType addUnrefDouble (InfoType elem);

    };

    typedef IRefInfo* RefInfo;
    
    
}
