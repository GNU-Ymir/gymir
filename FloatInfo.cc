#include <ymir/semantic/types/_.hh>

namespace semantic {

    using namespace syntax;

	
    IFloatInfo::IFloatInfo (bool isConst, FloatConst type) :
	IInfoType (isConst),
	_type (type)
    {}

    bool IFloatInfo::isSame (InfoType other) {
	if (auto ot = other-> to<IFloatInfo> ()) {
	    if (ot-> _type == this-> _type) return true;
	}
	return false;
    }
    
    InfoType IFloatInfo::BinaryOp (Word op, syntax::Expression right) {
	if (op == Token::EQUAL) return Affect (right);
	if (op == Token::DIV_AFF) return opAff (op, right);
	if (op == Token::STAR_EQUAL) return opAff (op, right);
	if (op == Token::MINUS_AFF) return opAff (op, right);
	if (op == Token::PLUS_AFF) return opAff (op, right);
	if (op == Token::INF) return opTest (op, right);
	if (op == Token::SUP) return opTest (op, right);
	if (op == Token::INF_EQUAL) return opTest (op, right);
	if (op == Token::SUP_EQUAL) return opTest (op, right);
	if (op == Token::NOT_EQUAL) return opTest (op, right);
	if (op == Token::NOT_INF_EQUAL) return opTest (op, right);
	if (op == Token::NOT_SUP_EQUAL) return opTest (op, right);
	if (op == Token::DEQUAL) return opTest (op, right);
	if (op == Token::PLUS) return opNorm (op, right);
	if (op == Token::MINUS) return opNorm (op, right);
	if (op == Token::DIV) return opNorm (op, right);
	if (op == Token::STAR) return opNorm (op, right);
	return NULL;
    } 
	
    InfoType IFloatInfo::BinaryOpRight (Word op, syntax::Expression left) {
	if (op == Token::EQUAL) return AffectRight (left);
	if (op == Token::INF) return opTestRight (op, left);
	if (op == Token::SUP) return opTestRight (op, left);
	if (op == Token::INF_EQUAL) return opTestRight (op, left);
	if (op == Token::SUP_EQUAL) return opTestRight (op, left);
	if (op == Token::NOT_EQUAL) return opTestRight (op, left);
	if (op == Token::DEQUAL) return opTestRight (op, left);
	if (op == Token::PLUS) return opNormRight (op, left);
	if (op == Token::MINUS) return opNormRight (op, left);
	if (op == Token::DIV) return opNormRight (op, left);
	if (op == Token::STAR) return opNormRight (op, left);
	return NULL;	
    }

    InfoType IFloatInfo::UnaryOp (Word op) {
	if (op == Token::MINUS) return Inv ();
	return NULL;
    }

    InfoType IFloatInfo::DotOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "init") return Init ();
	if (var-> token == "max") return Max ();
	if (var-> token == "min") return Min ();
	if (var-> token == "nan") return Nan ();
	if (var-> token == "dig") return Dig ();
	if (var-> token == "epsilon") return Epsilon ();
	if (var-> token == "mant_dig") return MantDig ();
	if (var-> token == "max_10_exp") return Max10Exp ();
	if (var-> token == "max_exp") return MaxExp ();
	if (var-> token == "min_10_exp") return Min10Exp ();
	if (var-> token == "min_exp") return MinExp ();
	if (var-> token == "typeid") return StringOf ();
	if (var-> token == "sqrt") return Sqrt ();
    }
	
    InfoType IFloatInfo::CastOp (InfoType other) {
	if (auto ot = other-> to<IFloatInfo> ()) {
	    if (ot-> _type == this-> _type) return this;
	    else {
		//TODO
		return other-> clone ();
	    }
	} else if (auto ot = other-> to<IFixedInfo> ()) {
	    //TODO
	    return ot-> clone ();
	}
    }

    InfoType IFloatInfo::CompOp (InfoType other) {
	if (other-> is<IUndefInfo> ()) {
	    //TODO
	    return this-> clone ();
	} else if (auto ot = other-> to<IFloatInfo> ()) {
	    if (this-> _type >= ot-> _type) {
		//TODO
		return this-> clone ();	       
	    }
	} else if (auto ref = other-> to<IRefInfo> ()) {
	    auto aux = new IRefInfo (this-> clone ());
	    //TODO
	    return aux;
	} else if (auto en = other-> to<IEnumInfo> ()) {
	    return this-> CompOp (en-> content ());
	}
	return NULL;
    }
	
    std::string IFloatInfo::innerTypeString () {
	if (this-> _type == FloatConst::FLOAT) return "float";
	else return "double";
    }

    std::string IFloatInfo::simpleTypeString () {
	if (this-> _type == FloatConst::FLOAT) return "f";
	else return "d";
    }

    InfoType IFloatInfo::clone () {
	return new IFloatInfo (this-> isConst (), this-> _type);
    }

    const char* IFloatInfo::getId () {
	return IFloatInfo::id ();
    }
	
    InfoType IFloatInfo::Affect (syntax::Expression right) {
	if (auto ot = right-> info-> type-> to<IFloatInfo> ()) {
	    if (this-> _type >= ot-> _type) {
		auto f = new IFloatInfo (false, this-> _type);
		//TODO
		return f;
	    } else if (right-> info-> type-> is<IFixedInfo> ()) {
		auto f = new IFloatInfo (false, this-> _type);
		//TODO
		return f;
	    }
	}
	return NULL;
    }

    InfoType IFloatInfo::AffectRight (syntax::Expression left) {
	if (left-> info-> type-> is<IUndefInfo> ()) {
	    auto fl = new IFloatInfo (false, this-> _type);
	    //TODO
	    return fl;
	}
	return NULL;
    }

    InfoType IFloatInfo::Inv () {
	//TODO
	return this-> cloneConst ();
    }

    InfoType IFloatInfo::Init () {
	//TODO
	return this-> cloneConst ();
    }

    InfoType IFloatInfo::Max () {
    	//TODO
	return this-> cloneConst ();
    }
	
    InfoType IFloatInfo::Min () {
    	//TODO
	return this-> cloneConst ();
    }

    InfoType IFloatInfo::Nan () {
    	//TODO
	return this-> cloneConst ();
    }

    InfoType IFloatInfo::Dig () {
    	//TODO
	return new IFixedInfo (true, FixedConst::UINT);	
    }

    InfoType IFloatInfo::Epsilon () {
    	//TODO
	return this-> cloneConst ();
    }

    InfoType IFloatInfo::MantDig () {
    	//TODO
	return new IFixedInfo (true, FixedConst::UINT);	
    }
	
    InfoType IFloatInfo::Max10Exp () {
    	//TODO
	return this-> cloneConst ();
    }

    InfoType IFloatInfo::MaxExp () {
    	//TODO
	return this-> cloneConst ();
    }

    InfoType IFloatInfo::MinExp () {
	//TODO
	return this-> cloneConst ();
    }

    InfoType IFloatInfo::Min10Exp () {
	//TODO
	return this-> cloneConst ();
    }

    InfoType IFloatInfo::Inf () {
    	//TODO
	return this-> cloneConst ();
    }

    InfoType IFloatInfo::Sqrt () {
	//TODO
	return this-> cloneConst ();
    }
    
    InfoType IFloatInfo::StringOf () {
	//TODO
	return new IStringInfo (true);
    }    

    InfoType IFloatInfo::opAff (Word op, syntax::Expression right) {
	if (auto ot = right-> info-> type-> to<IFloatInfo> ()) {
	    if (ot-> _type <= this-> _type) {
		//TODO
		return this-> clone ();
	    }
	}
	return NULL;
    }

    InfoType IFloatInfo::opNorm (Word op, syntax::Expression right) {
	if (auto ot = right-> info-> type-> to<IFloatInfo> ()) {
	    if (this-> _type >= ot->_type) {
		//TODO
		return this-> cloneConst ();
	    } else return ot-> cloneConst ();
	} else if (auto ot = right-> info-> type-> to<IFloatInfo> ()) {
	    auto fl = this-> cloneConst ();
	    return fl;
	}
	return NULL;
    }

    InfoType IFloatInfo::opTest (Word, syntax::Expression right) {
	if (auto ot = right-> info-> type-> to<IFloatInfo> ()) {
	    return new IBoolInfo (true);
	} else if (auto ot = right-> info-> type-> to<IFloatInfo> ()) {
	    return new IBoolInfo (true);
	}
	return NULL;
    }

    InfoType IFloatInfo::opNormRight (Word, syntax::Expression right) {
	if (auto ot = right-> info-> type-> to<IFloatInfo> ()) {
	    auto fl = this-> cloneConst ();
	    return fl;
	}
	return NULL;
    }

    InfoType IFloatInfo::opTestRight (Word, syntax::Expression right) {
	if (auto ot = right-> info-> type-> to<IFloatInfo> ()) {
	    return new IBoolInfo (true);
	}
	return NULL;
    }

    FloatConst IFloatInfo::type () {
	return this-> _type;
    }
    
   
}
