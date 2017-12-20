#pragma once

#include <ymir/semantic/types/InfoType.hh>
#include <ymir/ast/Constante.hh>

namespace semantic {

    class IFixedInfo : public IInfoType {

	FixedConst _type;
	
    public:

	IFixedInfo (bool isConst, FixedConst type);

	bool isSame (InfoType) override;

	static InfoType create (Word token, std::vector <syntax::Expression> templates) {
	    if (templates.size () != 0)
		Ymir::Error::notATemplate (token);

	    if (token == "byte") return new IFixedInfo (false, FixedConst::BYTE);
	    if (token == "ubyte") return new IFixedInfo (false, FixedConst::UBYTE);
	    if (token == "short") return new IFixedInfo (false, FixedConst::SHORT);
	    if (token == "ushort") return new IFixedInfo (false, FixedConst::USHORT);
	    if (token == "int") return new IFixedInfo (false, FixedConst::INT);
	    if (token == "uint") return new IFixedInfo (false, FixedConst::UINT);
	    if (token == "long") return new IFixedInfo (false, FixedConst::LONG);
	    if (token == "ulong") return new IFixedInfo (false, FixedConst::ULONG);
	    return NULL;
	}

	std::string innerTypeString () override;

	std::string simpleTypeString () override;
	
	InfoType BinaryOp (Word, syntax::Expression) override;

	InfoType BinaryOpRight (Word, syntax::Expression) override;
	
	InfoType UnaryOp (Word) override;

	InfoType CastOp (InfoType) override;
	
	InfoType CompOp (InfoType) override;

	//InfoType CastTo (InfoType) override;

	InfoType DotOp (syntax::Var) override;

	FixedConst type ();

	InfoType clone () override;
		
	static const char* id () {
	    return "IFixedInfo";
	}

	const char* getId () override;

	bool isSigned ();
	
	Ymir::Tree toGeneric () override;

	Ymir::Tree getInitFnPtr () override;
	
    private:

	InfoType toPtr ();

	InfoType pplus ();

	InfoType ssub ();

	InfoType Affect (syntax::Expression);

	InfoType AffectRight (syntax::Expression);

	InfoType opAff (Word, syntax::Expression);
	
	InfoType opTest (Word op, syntax::Expression);

	InfoType opNorm (Word op, syntax::Expression);

	InfoType Init ();

	InfoType Max ();

	InfoType Min ();

	InfoType SizeOf ();

	InfoType StringOf ();	
	
	bool isSup (IFixedInfo*);
	
    };

    typedef IFixedInfo* FixedInfo;
    
    
}
