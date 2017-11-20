#pragma once

#include <ymir/semantic/types/InfoType.hh>


namespace semantic {
    
    class IArrayInfo : public IInfoType {

	InfoType content;

    public:

	IArrayInfo (bool isConst, InfoType content);

	InfoType content ();

	bool isSame (InfoType other) override;

	static InfoType create (Word token, std::vector <syntax::Expression> templates) {
	    if (templates.size () != 1 || !(templates [0]-> is<Type> ())) {
		// if (auto cst = templates [0]-> info-> type-> to<StructCstInfo> ()) {}
		Ymir::Error::append (token,
				     Ymir::TakeAType,
				     "un", "type", ""
		);
	    } else {
		return new IArrayInfo (false, templates [0]-> info-> type);
	    }
	}

	InfoType BinaryOp (Word, syntax::Expression) override;

	InfoType BinaryOpRight (Word, syntax::Expression) override;

	InfoType ApplyOp (std::vector <syntax::Var> vars) override;
	
	InfoType AccessOp (Word, syntax::ParamList) override;

	InfoType DotOp (syntax::Var) override;

	InfoType clone () override;

	InfoType CastOp (InfoType) override;

	InfoType CompOp (InfoType) override;

	InfoType ConstVerif (InfoType) override;

	std::string innerTypeString () override;

	std::string simpleTypeString () override;

	InfoType getTemplate (ulong) override;

	static const char* id () {
	    return "IArrayInfo";
	}
	
	const char* getId () override;

    private:
	
	InfoType Is (Expression);

	InfoType NotIs (Expression);

	InfoType Affect (Expression);

	InfoType Ptr ();

	InfoType Length ();

	InfoType TypeId ();

	InfoType TupleOf ();

	InfoType Access (syntax::Expression);
	
    };

    typedef IArrayInfo* ArrayInfo;
    
}