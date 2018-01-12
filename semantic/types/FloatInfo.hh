#pragma once

#include <ymir/semantic/types/InfoType.hh>

namespace semantic {

    class IFloatInfo : public IInfoType {

	FloatConst _type;
	
    public:

	
	IFloatInfo (bool isConst, FloatConst type);

	FloatConst type ();
	
	static InfoType create (Word token, std::vector <syntax::Expression> tmps) {
	    if (tmps.size () != 0)
		Ymir::Error::notATemplate (token);

	    if (token == "float") return new IFloatInfo (false, FloatConst::FLOAT);
	    else if (token == "double") return new IFloatInfo (false, FloatConst::DOUBLE);
	    return NULL;
	}

	bool isSame (InfoType) override;

	InfoType BinaryOp (Word, syntax::Expression) override; 
	
	InfoType BinaryOpRight (Word, syntax::Expression) override;

	InfoType UnaryOp (Word) override;

	InfoType DotOp (syntax::Var) override;
	
	InfoType CastOp (InfoType) override;

	InfoType CompOp (InfoType) override;
	
	std::string innerTypeString () override;

	std::string innerSimpleTypeString () override;

	InfoType clone () override;

	Ymir::Tree toGeneric () override;
	
	static const char* id () {
	    return "IFloatInfo";
	}

	const char* getId () override;
	
    private:

	InfoType Affect (syntax::Expression);

	InfoType AffectRight (syntax::Expression);

	InfoType Inv ();

	InfoType Init ();

	InfoType Max ();
	
	InfoType Min ();

	InfoType Nan ();

	InfoType Dig ();

	InfoType Epsilon ();

	InfoType MantDig ();
	
	InfoType Max10Exp ();

	InfoType MaxExp ();

	InfoType MinExp ();

	InfoType Min10Exp ();

	InfoType Inf ();

	InfoType Sqrt ();

	InfoType StringOf ();

	InfoType opAff (Word, syntax::Expression);

	InfoType opNorm (Word, syntax::Expression);

	InfoType opTest (Word, syntax::Expression);

	InfoType opNormRight (Word, syntax::Expression);

	InfoType opTestRight (Word, syntax::Expression);
	
    };

    typedef IFloatInfo* FloatInfo;
    
}
