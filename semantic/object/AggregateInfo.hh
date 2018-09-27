#pragma once

#include <ymir/semantic/types/InfoType.hh>
#include <ymir/semantic/pack/Frame.hh>
#include <ymir/semantic/types/StructInfo.hh>
#include <ymir/semantic/types/TupleInfo.hh>

namespace syntax {
    class ITypedVar;
    typedef ITypedVar* TypedVar;

    class IExpression;
    typedef IExpression* Expression;

    class ITypeCreator;
    typedef ITypeCreator* TypeCreator;

    class ITypeAlias;
    typedef ITypeAlias* TypeAlias;
}

namespace semantic {

    class IFunctionInfo;    
    class IAggregateCstInfo;
    class IAggregateInfo;
    
    typedef IAggregateCstInfo* AggregateCstInfo;
    typedef IAggregateInfo* AggregateInfo;
    typedef IFunctionInfo* FunctionInfo;
    
    class IAggregateCstInfo : public IInfoType {

	Namespace _space;
	Word _locId;
	std::string _name;
	
	std::vector <FunctionInfo> _contrs;
	FunctionInfo _destr;
	std::vector <FunctionInfo> _methods;
	std::vector <FunctionInfo> _staticMeth;

	std::vector <syntax::Expression> _tmps;
	std::vector <syntax::Expression> _impl;
	bool _isUnion;
	bool _isExternal;
	bool _isFailure = false;
	bool _isOver;
	InfoType _info = NULL;

	std::vector <syntax::TypeAlias> _alias;
	
	syntax::TypeCreator _creator;
	IAggregateCstInfo* _anc = NULL;
	
    public :

	friend IAggregateInfo;
	
	IAggregateCstInfo (Word locId, Namespace space, std::string name, const std::vector <syntax::Expression> & tmps, const std::vector <syntax::Expression> & self, bool isUnion, bool isOver);
	
	std::vector <FunctionInfo> & getConstructors ();

	FunctionInfo& getDestructor ();

	std::vector <FunctionInfo> & getMethods ();

	std::vector <FunctionInfo> & getStaticMethods ();

	std::vector <syntax::TypeAlias> & getAlias ();
	
	Namespace space ();
	
	bool isSame (InfoType) override;

	InfoType onClone () override;
	
	InfoType DColonOp (syntax::Var) override;

	InfoType TempOp (const std::vector <::syntax::Expression> &) override;

	std::string typeString () override;

	std::string innerTypeString () override;

	std::string innerSimpleTypeString () override;

	Ymir::Tree toGeneric () override;

	bool isType () override;

	bool& isExtern ();
	
	std::string name ();

	bool isSuccessor (IAggregateCstInfo *info);
	
	syntax::TypeCreator& creator ();
	
	static const char * id () {
	    return "IAggregateCstInfo";
	}

	const char * getId () override;

	Word getLocId ();

    private :

	InfoType Init (::syntax::Var var);

	InfoType SizeOf ();

	TupleInfo constructImpl (); 

	bool recursiveGet (InfoType, InfoType);

	bool inPrivateContext ();

	bool inProtectedContext ();

	bool isMine (Namespace space);

	bool isProtectedForMe (Namespace space);
    };

    class IAggregateInfo : public IInfoType {

	Namespace _space;
	Word _locId;
	std::string _name;
	
	std::vector <FunctionInfo> _contrs;
	Frame _destr;
	std::vector <FunctionInfo> _methods;
	std::vector <FunctionInfo> _allMethods;
	std::vector <FunctionInfo> _staticMeth;
	
	std::vector <syntax::Expression> tmpsDone;
	AggregateCstInfo _id = NULL;
	AggregateInfo _anc = NULL;
	TupleInfo _impl;
	bool _isExternal;
	bool _static = false;
	bool _hasExemption = false;
	
	friend IAggregateCstInfo;

    public:
	
	IAggregateInfo (AggregateCstInfo from, Namespace space, std::string name, const std::vector <syntax::Expression> & tmpsDone, bool isExternal);

	bool isSame (InfoType) override;

	Frame getDestructor ();
	
	InfoType ConstVerif (InfoType) override;

	InfoType onClone () override;

	InfoType BinaryOp (Word, syntax::Expression) override;
	
	InfoType BinaryOpRight (Word, syntax::Expression) override;

	InfoType DotOp (syntax::Var) override;

	InfoType DotExpOp (syntax::Expression right) override;
	
	InfoType DColonOp (syntax::Var) override;

	InfoType CompOp (InfoType) override;

	std::string getName ();

	Namespace & getSpace ();

	std::string innerTypeString () override;

	std::string innerSimpleTypeString () override;

	TupleInfo getImpl ();
	
	Ymir::Tree getVtable ();
	
	Ymir::Tree toGeneric () override;	
	
	void setTmps (const std::vector <syntax::Expression> & tmps);

	InfoType getTemplate (ulong) override;

	std::vector <InfoType> getTemplate (ulong, ulong) override;

	AggregateInfo getAncestor ();

	bool& hasExemption ();
	
	Frame cpyCstr ();
	
	static const char * id () {
	    return "IAggregateInfo";
	}

	const char * getId () override;
	
    private :

	InfoType SizeOf ();

	InfoType Method (syntax::Var);

	InfoType Super ();

	InfoType Init (::syntax::Var);
	
	//InfoType Name ();

	Ymir::Tree buildVtableType ();
	
	Ymir::Tree buildVtableEnum (Ymir::Tree vtype);

	std::vector <FunctionInfo> getMethods ();

	bool inPrivateContext ();

	bool inProtectedContext ();

	bool isMine (Namespace space);

	bool isProtectedForMe (Namespace space);

	bool hasCopyCstr ();
	
	InfoType BinaryOpRightCpy (Word, syntax::Expression, bool);

	InfoType AliasOp (syntax::Var var);
	
    };

}
