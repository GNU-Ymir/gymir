#pragma once

#include <ymir/semantic/types/InfoType.hh>
#include <ymir/ast/Constante.hh>

namespace semantic {

    class IFixedInfo : public IInfoType {

	FixedConst _type;
	
    public:

	IFixedInfo (bool isConst, FixedConst type);

	bool isSame (InfoType) override;

	static InfoType create (Word token, const std::vector<syntax::Expression> & templates) {
	    if (templates.size () != 0)
		Ymir::Error::notATemplate (token);

	    if (token == "byte") return new (Z0) IFixedInfo (false, FixedConst::BYTE);
	    if (token == "ubyte") return new (Z0) IFixedInfo (false, FixedConst::UBYTE);
	    if (token == "short") return new (Z0) IFixedInfo (false, FixedConst::SHORT);
	    if (token == "ushort") return new (Z0) IFixedInfo (false, FixedConst::USHORT);
	    if (token == "int") return new (Z0) IFixedInfo (false, FixedConst::INT);
	    if (token == "uint") return new (Z0) IFixedInfo (false, FixedConst::UINT);
	    if (token == "long") return new (Z0) IFixedInfo (false, FixedConst::LONG);
	    if (token == "ulong") return new (Z0) IFixedInfo (false, FixedConst::ULONG);
	    return NULL;
	}

	std::string innerTypeString () override;

	std::string innerSimpleTypeString () override;
	
	InfoType BinaryOp (Word, syntax::Expression) override;

	InfoType BinaryOpRight (Word, syntax::Expression) override;
	
	InfoType UnaryOp (Word) override;

	InfoType CastOp (InfoType) override;
	
	InfoType CompOp (InfoType) override;

	//InfoType CastTo (InfoType) override;

	InfoType DotOp (syntax::Var) override;
	
	InfoType DColonOp (syntax::Var) override;

	FixedConst type ();	   
	
	InfoType onClone () override;
		
	static const char* id () {
	    return "IFixedInfo";
	}

	const char* getId () override;

	bool isSigned ();
	
	Ymir::Tree toGeneric () override;
	
    private:

	InfoType toPtr ();

	InfoType pplus ();

	InfoType ssub ();

	InfoType Affect (syntax::Expression);

	InfoType AffectRight (syntax::Expression);

	InfoType opAff (Word, syntax::Expression);
	
	InfoType opReaff (Word, syntax::Expression);

	InfoType opRange (Word op, syntax::Expression);
	
	InfoType opTest (Word op, syntax::Expression);

	InfoType opNorm (Word op, syntax::Expression);

	InfoType Init ();

	InfoType Max ();

	InfoType Min ();

	InfoType SizeOf ();
	
	bool isSup (IFixedInfo*);
	
    };

    typedef IFixedInfo* FixedInfo;
    
    
}
