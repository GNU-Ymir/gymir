#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace syntax {
    class ITypedVar;
    typedef ITypedVar* TypedVar;

    class IExpression;
    typedef IExpression* Expression;    
}

namespace semantic {

    class IStructCstInfo : public IInfoType {

	Namespace space;

	std::string name;

	std::vector <syntax::TypedVar> params;

	std::vector <syntax::Expression> tmps;
	
	bool _isPublic;
	
    public:

	IStructCstInfo (Namespace space, std::string name, std::vector <syntax::Expression> & tmps);

	bool isSame (InfoType) override;
	
	InfoType onClone () override;

	InfoType DotOp (syntax::Var) override;

	InfoType DColonOp (syntax::Var) override;

	ApplicationScore CallOp (Word, syntax::ParamList) override;
	
	ApplicationScore CallOp (Word, const std::vector<InfoType> &) override;

	InfoType TempOp (const std::vector<::syntax::Expression> &) override;
	
	std::string innerTypeString () override;

	std::string innerSimpleTypeString () override;

	bool isType () override;
	
	void isPublic (bool);

	void addAttrib (syntax::TypedVar param);
	
	static const char* id () {
	    return "IStructCstInfo";
	}

	const char* getId () override;

    };    
    
    class IStructInfo : public IInfoType {

	Namespace space;
	std::string name;
	std::vector <InfoType> types;
	std::vector <std::string> attrs;

    public:

	IStructInfo (Namespace space, std::string name);

	bool isSame (InfoType) override;

	InfoType BinaryOp (Word op, syntax::Expression left) override;
	
	InfoType BinaryOpRight (Word op, syntax::Expression left) override;
	
	InfoType onClone () override;

	InfoType DotOp (syntax::Var) override;

	InfoType DColonOp (syntax::Var) override;
	
	InfoType CompOp (InfoType) override;
	
	std::string innerTypeString () override;

	std::string innerSimpleTypeString () override;

	Ymir::Tree toGeneric () override;

	void setTypes (std::vector <InfoType> types);

	void setAttribs (std::vector <std::string> names);

	std::vector <std::string> & getAttribs ();
	
	static const char* id () {
	    return "IStructInfo";
	}

	const char* getId () override;	
    };

    typedef IStructInfo* StructInfo;
    typedef IStructCstInfo* StructCstInfo;
    
}
