#pragma once

#include <ymir/semantic/types/InfoType.hh>
#include <ymir/ast/Expression.hh>
#include <ymir/ast/Type.hh>
#include <ymir/utils/Array.hh>

namespace semantic {
    
    class IArrayInfo : public IInfoType {

	InfoType _content;

    public:

	IArrayInfo (bool isConst, InfoType content);

	InfoType content ();

	bool isSame (InfoType other) override;

	static InfoType create (Word token, const std::vector<syntax::Expression> & templates) {
	    if (templates.size () != 1 || !(templates [0]-> is<syntax::IType> ())) {
		// if (auto cst = templates [0]-> info-> type-> to<StructCstInfo> ()) {}
		Ymir::Error::takeATypeAsTemplate (token);		
	    } else {
		return new  IArrayInfo (false, templates [0]-> info-> type);
	    }
	}

	InfoType BinaryOp (Word, syntax::Expression) override;

	InfoType BinaryOpRight (Word, syntax::Expression) override;

	InfoType ApplyOp (const std::vector<syntax::Var> & vars) override;
	
	InfoType AccessOp (Word, syntax::ParamList, std::vector <InfoType> &) override;

	InfoType DotOp (syntax::Var) override;

	InfoType onClone () override;

	InfoType CastOp (InfoType) override;

	InfoType CompOp (InfoType) override;

	InfoType ConstVerif (InfoType) override;

	std::string innerTypeString () override;

	std::string innerSimpleTypeString () override;

	Ymir::Tree toGeneric () override;
	
	InfoType getTemplate (ulong) override;
       	
	static const char* id () {
	    return "IArrayInfo";
	}
	
	const char* getId () override;

    private:
	
	InfoType Is (syntax::Expression);

	InfoType NotIs (syntax::Expression);

	InfoType Affect (syntax::Expression);

	InfoType AffectRight (syntax::Expression);

	InfoType Ptr ();

	InfoType Length ();

	InfoType TypeId ();

	InfoType TupleOf ();

	InfoType Access (syntax::Expression, InfoType&);
	
	InfoType Concat (syntax::Expression);

	InfoType ConcatAff (syntax::Expression);

    };

    typedef IArrayInfo* ArrayInfo;
    
}
