#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace syntax {
    class ITypedVar;
    typedef ITypedVar* TypedVar;

    class IExpression;
    typedef IExpression* Expression;    
}

namespace semantic {

    class IStructInfo;
    typedef IStructInfo* StructInfo;
    
    class IStructCstInfo : public IInfoType {

	Namespace space;

	Word _locId;
	
	std::string name;

	std::vector <syntax::TypedVar> params;

	std::vector <syntax::Expression> tmps;

	std::vector <syntax::Expression> tmpsDone;
	
	StructInfo _info;
	
	bool _isPublic;
	
    public:

	IStructCstInfo (Word locId, Namespace space, std::string name, std::vector <syntax::Expression> & tmps);

	bool isSame (InfoType) override;

	bool isInstance (StructInfo info);
	
	InfoType onClone () override;
	
	InfoType DotOp (syntax::Var) override;

	InfoType DColonOp (syntax::Var) override;

	ApplicationScore CallOp (Word, syntax::ParamList) override;
	
	ApplicationScore CallOp (Word, const std::vector<InfoType> &) override;

	InfoType TempOp (const std::vector<::syntax::Expression> &) override;

	std::string typeString () override;

	std::string onlyNameTypeString ();
	
	std::string innerTypeString () override;

	std::string innerSimpleTypeString () override;

	Ymir::Tree toGeneric () override;

	bool isType () override;
	
	void isPublic (bool);

	void addAttrib (syntax::TypedVar param);
		
	static const char* id () {
	    return "IStructCstInfo";
	}

	const char* getId () override;

	Word getLocId ();
	
    private:

	InfoType getScore (const std::vector <syntax::Expression> & tmps);

	bool recursiveGet (InfoType info, InfoType where);
	
	InfoType Init ();

	InfoType SizeOf ();
	
    };    
    
    class IStructInfo : public IInfoType {

	Namespace space;
	std::string name;
	std::vector <InfoType> types;
	std::vector <std::string> attrs;
	std::vector <syntax::Expression> tmpsDone;
	IStructCstInfo* _id = NULL;
	
	friend IStructCstInfo;

    public:
	
	IStructInfo (IStructCstInfo* from, Namespace space, std::string name);

	bool isSame (InfoType) override;
	
	InfoType ConstVerif (InfoType) override;

	ApplicationScore CallOp (Word, syntax::ParamList) override;
	
	InfoType BinaryOp (Word op, syntax::Expression left) override;
	
	InfoType BinaryOpRight (Word op, syntax::Expression left) override;
	
	InfoType onClone () override;

	InfoType DotOp (syntax::Var) override;

	InfoType DColonOp (syntax::Var) override;
	
	InfoType CompOp (InfoType) override;

	InfoType UnaryOp (Word) override;

	std::string getName ();

	Namespace & getSpace ();

	std::string onlyNameTypeString ();
	
	std::string innerTypeString () override;
	
	std::string innerSimpleTypeString () override;

	Ymir::Tree toGeneric () override;

	void setTypes (std::vector <InfoType> types);

	void setAttribs (std::vector <std::string> names);

	void setTmps (std::vector <syntax::Expression> tmps);

	InfoType getTemplate (ulong) override;

	std::vector <InfoType> getTemplate (ulong, ulong) override;
	
	std::vector <std::string> & getAttribs ();

	std::vector <InfoType> & getTypes ();

	InfoType StringOf () override;
	
	static const char* id () {
	    return "IStructInfo";
	}

	const char* getId () override;
	
    private:

	InfoType Init ();

	InfoType SizeOf ();	
	
    };

    typedef IStructInfo* StructInfo;
    typedef IStructCstInfo* StructCstInfo;
    
}
