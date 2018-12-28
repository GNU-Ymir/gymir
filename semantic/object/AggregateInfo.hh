#pragma once

#include <ymir/semantic/types/InfoType.hh>
#include <ymir/semantic/pack/Frame.hh>
#include <ymir/semantic/types/StructInfo.hh>
#include <ymir/semantic/types/TupleInfo.hh>

namespace syntax {

    enum class InnerProtection;
    // 	PUBLIC,
    // 	PRIVATE,
    // 	PROTECTED
    // };

    class ITypedVar;
    typedef ITypedVar* TypedVar;

    class IExpression;
    typedef IExpression* Expression;

    class ITypeCreator;
    typedef ITypeCreator* TypeCreator;

    class ITypeAttr;
    typedef ITypeAttr* TypeAttr;

    class IBlock;
    typedef IBlock* Block;
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
	syntax::Block _staticBlock;
	
	std::vector <syntax::Expression> _tmps;
	std::vector <syntax::Expression> _tmpsDone;
	syntax::Expression _ancExpr;

	std::vector <std::vector <syntax::Expression>> _solvedTmps;
	std::vector <InfoType> _solved;
	
	bool _isExternal;
	bool _isFailure = false;
	bool _isDynamic = false;
	IAggregateInfo* _info = NULL;

	std::vector <syntax::TypeAttr> _attrs;
	std::vector <syntax::TypeAttr> _staticAttrs;
	
	syntax::TypeCreator _creator;
	Namespace _templateSpace;
	
	IAggregateCstInfo* _anc = NULL;
	
    public :

	friend IAggregateInfo;
	
	IAggregateCstInfo (Word locId, Namespace space, std::string name, const std::vector <syntax::Expression> & tmps, syntax::Expression over, const std::vector <Word> & udas);
	
	std::vector <FunctionInfo> & getConstructors ();

	FunctionInfo& getDestructor ();

	std::vector <FunctionInfo> & getMethods ();

	FunctionInfo getMeth (std::string name);
	
	std::vector <FunctionInfo> & getStaticMethods ();

	std::vector <syntax::TypeAttr> & getStaticVars ();

	syntax::Block & getStaticBlock ();
	
	FunctionInfo getStaticMeth (std::string name);
	
	std::vector <syntax::TypeAttr> & getAttrs ();
	
	Namespace space ();

	bool& isDynamic ();
	
	Namespace & templateSpace ();

	bool isSame (InfoType) override;
	
	InfoType onClone () override;
	
	InfoType DColonOp (syntax::Var) override;

	InfoType TempOp (const std::vector <::syntax::Expression> &) override;
	
	std::string typeString () override;

	std::string simpleTypeString () override;

	std::string innerTypeString () override;

	std::string innerSimpleTypeString () override;

	Ymir::Tree toGeneric () override;
	
	bool isType () override;

	bool& isExtern ();
	
	std::string name ();

	bool isSuccessor (IAggregateCstInfo *info);
	
	syntax::TypeCreator& creator ();

	std::vector <syntax::Expression> & tmpsDone ();
	
	static const char * id () {
	    return "IAggregateCstInfo";
	}

	const char * getId () override;

	Word getLocId ();

    private :

	InfoType Init (::syntax::Var var);

	InfoType SizeOf ();

	bool recursiveGet (InfoType, InfoType);

	bool inPrivateContext ();

	bool inProtectedContext ();

	bool isMine (Namespace space);

	bool isProtectedForMe (Namespace space);

	InfoType getScore (const std::vector <syntax::Expression>&);

	InfoType findAlreadySolve (std::vector <syntax::Expression>&);

	void addAlreadySolve (std::vector <syntax::Expression>&, InfoType);
	
    };

    class IAggregateInfo : public IInfoType {

	Namespace _space;
	Word _locId;
	std::string _name;
	
	std::vector <FunctionInfo> _contrs;
	Frame _destr;
	std::vector <FunctionInfo> _methods;
	std::vector <FunctionInfo> _allMethods;
	
	std::vector <std::string> _attrs;
	std::vector <Namespace> _attrSpaces;
	std::vector <syntax::InnerProtection> _prots;
	
	std::vector <InfoType> _types;

	std::vector <FunctionInfo> _staticMeth;
	
	std::vector <syntax::Expression> tmpsDone;
	AggregateCstInfo _id = NULL;
	AggregateInfo _anc = NULL;
	
	bool _isExternal;
	bool _static = false;
	bool _isDynamic = false;

	friend IAggregateCstInfo;

    public:
	
	IAggregateInfo (AggregateCstInfo from, Namespace space, std::string name, const std::vector <syntax::Expression> & tmpsDone, bool isExternal);

	bool isSame (InfoType) override;

	Frame getDestructor ();
	
	InfoType ConstVerif (InfoType) override;

	bool needKeepConst () override;
	
	InfoType onClone () override;

	InfoType BinaryOp (Word, syntax::Expression) override;
	
	InfoType BinaryOpRight (Word, syntax::Expression) override;

	InfoType UnaryOp (Word op);
	
	InfoType DotOp (syntax::Var) override;
	
	InfoType DColonOp (syntax::Var) override;

	InfoType CompOp (InfoType) override;

	InfoType CastOp (InfoType) override;

	std::string getName ();

	Namespace & getSpace ();

	std::string innerTypeString () override;

	std::string innerSimpleTypeString () override;

	Ymir::Tree getVtable ();
	
	Ymir::Tree toGeneric () override;

	Ymir::Tree genericDynInner ();

	Ymir::Tree genericCstDynInner ();
	
	Ymir::Tree genericConstructor () override;	

	Ymir::Tree genericTypeInfo () override;
	
	void setTmps (const std::vector <syntax::Expression> & tmps);

	InfoType getTemplate (ulong) override;

	std::vector <InfoType> getTemplate (ulong, ulong) override;

	std::vector <FunctionInfo> getAllMethods ();
	
	AggregateInfo getAncestor ();

	std::vector <InfoType>& getTypes ();

	std::vector <std::string> & getAttrs ();

	std::vector <syntax::InnerProtection>& getInnerProts ();

	std::vector <Namespace> & getAttrSpaces ();

	bool& isDynamic ();

	bool isMutable () override;
	
	InfoType isTyped (IAggregateInfo*);
		
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

	
    };

}
