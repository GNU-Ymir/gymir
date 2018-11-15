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

	std::vector <Word> _udas;

	bool _isUnion;
	
	StructInfo _info;
	
	bool _isPublic;

	friend IStructInfo;
	
    public:

	IStructCstInfo (Word locId, Namespace space, std::string name, std::vector <syntax::Expression> & tmps, std::vector <Word> attrs, bool isUnion);

	bool isSame (InfoType) override;

	bool isInstance (StructInfo info);
	
	InfoType onClone () override;
	
	InfoType DotOp (syntax::Var) override;

	InfoType DColonOp (syntax::Var) override;

	ApplicationScore CallOpUnion (Word, syntax::ParamList);
	
	ApplicationScore CallType (Word, syntax::ParamList) override;

	// ApplicationScore CallOpUnion (Word, const std::vector <InfoType> &);
	
	// ApplicationScore CallOp (Word, const std::vector<InfoType> &) override;

	InfoType TempOp (const std::vector<::syntax::Expression> &) override;

	std::vector <Word> &udas ();
	
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
	
	InfoType Init (syntax::Var var);

	InfoType Name ();

	InfoType SizeOf ();
	
    };    
    
    class IStructInfo : public IInfoType {

	Namespace space;
	std::string name;
	std::vector <InfoType> types;
	std::vector <std::string> attrs;
	std::vector <syntax::Expression> tmpsDone;
	IStructCstInfo* _id = NULL;
	std::vector <Word> _udas;
	bool _isUnion;

	friend IStructCstInfo;

    public:
	
	IStructInfo (IStructCstInfo* from, Namespace space, std::string name, std::vector <Word> udas, bool isUnion);

	bool isSame (InfoType) override;
	
	InfoType ConstVerif (InfoType) override;

	bool passingConst (InfoType) override;
	
	ApplicationScore CallType (Word, syntax::ParamList) override;
	
	InfoType BinaryOp (Word op, syntax::Expression left) override;
	
	InfoType BinaryOpRight (Word op, syntax::Expression left) override;
	
	InfoType onClone () override;

	InfoType DotOp (syntax::Var) override;

	InfoType DColonOp (syntax::Var) override;
	
	InfoType CompOp (InfoType) override;

	InfoType UnaryOp (Word) override;

	std::string getName ();

	Namespace & getSpace ();

	std::string onlyNameTypeString (bool simple = true);
	
	std::string innerTypeString () override;
	
	std::string innerSimpleTypeString () override;

	Ymir::Tree toGeneric () override;

	Ymir::Tree genericConstructor () override;
	
	void setTypes (std::vector <InfoType> types);

	void setAttribs (std::vector <std::string> names);

	void setTmps (std::vector <syntax::Expression> tmps);

	InfoType getTemplate (ulong) override;

	std::vector <InfoType> getTemplate (ulong, ulong) override;
	
	std::vector <std::string> & getAttribs ();

	std::vector <InfoType> & getTypes ();

	InfoType typeOfAttrib (const std::string & name);

	bool needKeepConst () override;
	
	std::vector <Word> &udas ();

	bool has (std::string attr);
	
	InfoType StringOf () override;
	
	static const char* id () {
	    return "IStructInfo";
	}

	const char* getId () override;
	
    private:

	InfoType Init ();

	InfoType SizeOf ();	
	
	InfoType Name ();
    };

    typedef IStructInfo* StructInfo;
    typedef IStructCstInfo* StructCstInfo;
    
}
