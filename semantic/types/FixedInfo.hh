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

	    if (token == "i8") return new (Z0) IFixedInfo (false, FixedConst::BYTE);
	    if (token == "u8") return new (Z0) IFixedInfo (false, FixedConst::UBYTE);
	    if (token == "i16") return new (Z0) IFixedInfo (false, FixedConst::SHORT);
	    if (token == "u16") return new (Z0) IFixedInfo (false, FixedConst::USHORT);
	    if (token == "i32") return new (Z0) IFixedInfo (false, FixedConst::INT);
	    if (token == "u32") return new (Z0) IFixedInfo (false, FixedConst::UINT);
	    if (token == "i64") return new (Z0) IFixedInfo (false, FixedConst::LONG);
	    if (token == "u64") return new (Z0) IFixedInfo (false, FixedConst::ULONG);
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

	Ymir::Tree genericConstructor () override;

	Ymir::Tree genericTypeInfo () override;
	
    private:

	InfoType toPtr (Word & tok);

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
