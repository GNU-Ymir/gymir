#pragma once

#include <ymir/semantic/types/InfoType.hh>
#include <ymir/ast/Type.hh>

namespace semantic {

    class IArrayRefInfo : public IInfoType {

	InfoType _content = NULL;

    public:

	IArrayRefInfo (bool);

	IArrayRefInfo (bool, InfoType);

	bool isSame (InfoType) override;

	InfoType ConstVerif (InfoType) override;

	InfoType BinaryOp (Word token, syntax::Expression right) override;

	InfoType BinaryOpRight (Word token, syntax::Expression left) override;

	InfoType AccessOp (Word token, syntax::ParamList params, std::vector <InfoType> &) override;

	InfoType DotOp (syntax::Var var) override;

	InfoType DotExpOp (syntax::Expression elem) override;

	InfoType DColonOp (syntax::Var var) override;

	InfoType UnaryOp (Word op) override;

	InfoType CastOp (InfoType other) override;

	InfoType CompOp (InfoType other) override;

	//InfoType ApplyOp (const std::vector<syntax::Var> & vars) override;

	ApplicationScore CallOp (Word op, syntax::ParamList params) override;
	
	std::string innerTypeString () override;
	
	std::string innerSimpleTypeString () override;
	
	InfoType onClone () override;
	
	static const char* id () {
	    return "IArrayRefInfo";
	}

	Ymir::Tree toGeneric () override;
	
	const char* getId () override;

	InfoType content ();
	
    };

    typedef IArrayRefInfo* ArrayRefInfo;
    
    
}
