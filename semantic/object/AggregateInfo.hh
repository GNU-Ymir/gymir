#pragma once

#include <ymir/semantic/types/InfoType.hh>
#include <ymir/semantic/pack/Frame.hh>
#include <ymir/semantic/types/StructInfo.hh>

namespace syntax {
    class ITypedVar;
    typedef ITypedVar* TypedVar;

    class IExpression;
    typedef IExpression* Expression;    
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
	
    public :

	IAggregateCstInfo (Word locId, Namespace space, std::string name, const std::vector <syntax::Expression> & tmps, const std::vector <syntax::Expression> & self, bool isUnion);
	
	std::vector <FunctionInfo> & getConstructors ();

	FunctionInfo& getDestructor ();

	std::vector <FunctionInfo> & getMethods ();

	std::vector <FunctionInfo> & getStaticMethods ();

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

	std::string name ();
	
	static const char * id () {
	    return "IAggregateCstInfo";
	}

	const char * getId () override;

	Word getLocId ();

    private :

	InfoType Init ();

	InfoType SizeOf ();
	
    };

    class IAggregateInfo : public IInfoType {

	Namespace _space;
	Word _locId;
	std::string _name;
	
	std::vector <FunctionInfo> _contrs;
	Frame _destr;
	std::vector <FunctionInfo> _methods;
	std::vector <FunctionInfo> _staticMeth;
	
	std::vector <syntax::Expression> tmpsDone;
	AggregateCstInfo _id = NULL;
	StructInfo _impl;

	friend IAggregateCstInfo;

    public:
	
	IAggregateInfo (AggregateCstInfo from, Namespace space, std::string name, const std::vector <syntax::Expression> & tmpsDone);

	bool isSame (InfoType) override;

	Frame getDestructor ();
	
	InfoType ConstVerif (InfoType) override;

	InfoType onClone () override;

	InfoType BinaryOp (Word, syntax::Expression) override;
	
	InfoType BinaryOpRight (Word, syntax::Expression) override;

	InfoType DotOp (syntax::Var) override;

	InfoType DColonOp (syntax::Var) override;

	InfoType CompOp (InfoType) override;

	std::string getName ();

	Namespace & getSpace ();

	std::string innerTypeString () override;

	std::string innerSimpleTypeString () override;

	Ymir::Tree getVtable ();
	
	Ymir::Tree toGeneric () override;

	void setTmps (const std::vector <syntax::Expression> & tmps);

	InfoType getTemplate (ulong) override;

	std::vector <InfoType> getTemplate (ulong, ulong) override;

	static const char * id () {
	    return "IAggregateInfo";
	}

	const char * getId () override;
	
    private :

	InfoType SizeOf ();

	InfoType Method (syntax::Var);
	
	//InfoType Name ();

	Ymir::Tree buildVtableType ();
	
	Ymir::Tree buildVtableEnum (Ymir::Tree vtype);
	
    };

}
