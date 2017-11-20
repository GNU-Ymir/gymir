#include <ymir/semantic/types/_.hh>

namespace semantic {

    using namespace syntax;
    
    IFixedInfo::IFixedInfo (bool isConst, FixedConst type) :
	IInfoType (isConst),
	_type (type)
    {}

    bool IFixedInfo::isSame (InfoType other) {
	auto	ot		= other-> to<IFixedInfo> ();
	if (ot && ot-> type () == this-> type ()) {
	    return true;
	}
	return false;
    }
    
    std::string IFixedInfo::innerTypeString () {
	return syntax::name (this-> _type);
    }

    std::string IFixedInfo::simpleTypeString () {
	return syntax::sname (this-> _type);
    }
	
    InfoType IFixedInfo::BinaryOp (Word op, syntax::Expression right) {
	if (op == Token::EQUAL) return Affect (right);
	if (op == Token::DIV_AFF) return opAff (op, right);
	if (op == Token::AND_AFF) return opAff (op, right);
	if (op == Token::PIPE_EQUAL) return opAff (op, right);
	if (op == Token::MINUS_AFF) return opAff (op, right);
	if (op == Token::PLUS_AFF) return opAff (op, right);
	if (op == Token::LEFTD_AFF) return opAff (op, right);
	if (op == Token::RIGHTD_AFF) return opAff (op, right);
	if (op == Token::STAR_EQUAL) return opAff (op, right);
	if (op == Token::PERCENT_EQUAL) return opAff (op, right);
	if (op == Token::XOR_EQUAL) return opAff (op, right);
	if (op == Token::DAND) return opNorm (op, right);
	if (op == Token::DPIPE) return opNorm (op, right);
	if (op == Token::INF) return opTest (op, right);
	if (op == Token::SUP) return opTest (op, right);
	if (op == Token::INF_EQUAL) return opTest (op, right);
	if (op == Token::SUP_EQUAL) return opTest (op, right);
	if (op == Token::NOT_EQUAL) return opTest(op, right);
	if (op == Token::NOT_INF) return opTest(op, right);
	if (op == Token::NOT_INF_EQUAL) return opTest(op, right);
	if (op == Token::NOT_SUP) return opTest(op, right);
	if (op == Token::NOT_SUP_EQUAL) return opTest(op, right);
	if (op == Token::DEQUAL) return opTest(op, right);
	if (op == Token::PLUS) return opNorm (op, right);
	if (op == Token::MINUS) return opNorm (op, right);
	if (op == Token::DIV) return opNorm (op, right);
	if (op == Token::STAR) return opNorm (op, right);
	if (op == Token::PIPE) return opNorm(op, right);
	if (op == Token::AND) return opNorm(op, right);
	if (op == Token::LEFTD) return opNorm(op, right);
	if (op == Token::XOR) return opNorm(op, right);
	if (op == Token::RIGHTD) return opNorm(op, right);
	if (op == Token::PERCENT) return opNorm(op, right);
	return NULL;
    }

    InfoType IFixedInfo::BinaryOpRight (Word op, syntax::Expression left) {
	if (op == Token::EQUAL) return AffectRight (left);
	return NULL;
    }
	
    InfoType IFixedInfo::UnaryOp (Word op) {
	if (op == Token::MINUS) {
	    auto ret = new IFixedInfo (true, this-> type ());
	    //TODO
	    return ret;
	} else if (op == Token::AND && !this-> isConst ())
	    return toPtr ();
	return NULL;
    }

    InfoType IFixedInfo::CastOp (InfoType other) {
	if (this-> isSame (other)) return this;
	if (other-> is<IBoolInfo> ()) {
	    auto aux = new IBoolInfo (this-> isConst ());
	    //TODO
	    return aux;
	} else if (other-> is<ICharInfo> ()) {
	    auto aux = new ICharInfo (this-> isConst ());
	    //TODO
	    return aux;
	} else if (auto ot = other-> to<IFloatInfo> ()) {
	    auto aux = new IFloatInfo (this-> isConst (), ot-> type ());
	    //TODO
	    return aux;
	} else if (auto ot = other-> to<IFixedInfo> ()) {
	    auto ret = ot-> clone ();
	    ret-> isConst () = this-> isConst ();
	    //TODO
	    return ret;
	}
	return NULL;
    }
	
    InfoType IFixedInfo::CompOp (InfoType other) {
	if (other-> is<IUndefInfo> () || this-> isSame (other)) {
	    auto ret = this-> clone ();
	    //TODO
	    return ret;
	} else if (auto ref = other-> to<IRefInfo> ()) {
	    if (!this-> isConst () && this-> isSame (ref-> content ())) {
		auto aux = new IRefInfo (this-> clone ());
		//TODO
		return aux;
	    }
	} else if (auto ot = other-> to<IFixedInfo> ()) {
	    if (this-> isSigned () && ot-> isSigned () && ot-> isSup (this)) {
		auto ret = this-> clone ();
		//TODO;
		return ret;
	    } else if (!this-> isSigned () && !ot-> isSigned () && ot-> isSup (this)) {
		auto ret = this-> clone ();
		//TODO;
		return ret;
	    }
	} else if (auto ot = other-> to<IEnumInfo> ()) {
	    return this->CompOp (ot-> content ());
	}
	return NULL;
    }

    InfoType IFixedInfo::DotOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "init") return Init ();
	if (var-> token == "max") return Max ();
	if (var-> token == "min") return Min ();
	if (var-> token == "sizeof") return SizeOf ();
	if (var-> token == "typeid") return StringOf ();
	return NULL;
    }

    FixedConst	IFixedInfo::type () {
	return this-> _type;
    }

    InfoType IFixedInfo::clone () {
	auto ret = new IFixedInfo (this-> isConst (), this-> _type);
	//TODO
	return ret;
    }

    const char* IFixedInfo::getId () {
	return IFixedInfo::id ();
    }
	
    InfoType IFixedInfo::toPtr () {
	auto ret = new IPtrInfo (this-> isConst (), this-> clone ());
	//TODO
	return ret;
    }

    InfoType IFixedInfo::pplus () {
	auto ret = this-> clone ();
	//TODO
	return ret;
    }

    InfoType IFixedInfo::ssub () {
	auto ret = this-> clone ();
	//TODO
	return ret;
    }

    InfoType IFixedInfo::Affect (syntax::Expression right) {
	if (auto ot = right-> info-> type-> to<IFixedInfo> ()) {
	    if (this-> isSigned () && ot-> isSigned () && this-> isSup (ot)) {
		auto ret = this-> clone ();
		//TODO
		return ret;
	    } else if (!this-> isSigned () && !ot-> isSigned () && this-> isSup (ot)) {
		auto ret = this-> clone ();
		//TODO
		return ret;
	    } else if (this-> _type == ot-> type ()) {
		auto ret = this-> clone ();
		//TODO
		return ret;
	    }
	}
	return NULL;
    }

    InfoType IFixedInfo::AffectRight (syntax::Expression left) {
	if (left-> info-> type-> is<IUndefInfo> ()) {
	    auto i = new IFixedInfo (false, this-> _type);
	    //TODO;
	    return i;
	}
	return NULL;
    }

    InfoType IFixedInfo::opAff (Word op, syntax::Expression right) {
	if (auto ot = right-> info-> type-> to<IFixedInfo> ()) {
	    if (this-> isSigned () && ot-> isSigned () && this-> isSup (ot)) {
		auto ret = this-> clone ();
		//TODO
		return ret;
	    } else if (!this-> isSigned () && !ot-> isSigned () && this-> isSup (ot)) {
		auto ret = this-> clone ();
		//TODO
		return ret;
	    } else if (this-> _type == ot-> type ()) {
		auto ret = this-> clone ();
		//TODO
		return ret;
	    }
	}
	return NULL;
    }
	
    InfoType IFixedInfo::opTest (Word op, syntax::Expression right) {
	if (auto ot = right-> info-> type-> to<IFixedInfo> ()) {
	    if (this-> _type == ot-> type ()) {
		auto ret = new IBoolInfo (true);
		//TODO
		return ret;	    
	    } else if (this-> isSigned () && ot-> isSigned ()) {
		if (this-> isSup (ot)) {
		    auto ret = new IBoolInfo (true);
		    //TODO
		    return ret;
		} else {
		    auto ret = new IBoolInfo (true);
		    //TODO
		    return ret;
		}
	    } else if (!this-> isSigned () && !ot-> isSigned ()) {
		if (this-> isSup (ot)) {
		    auto ret = new IBoolInfo (true);
		    //TODO
		    return ret;
		} else {
		    auto ret = new IBoolInfo (true);
		    //TODO
		    return ret;
		}
	    }
	} else if (auto ot = right-> info-> type-> to<ICharInfo> ()) {
	    if (this-> _type == FixedConst::UBYTE) {
		auto ret = new IBoolInfo (true);
		//TODO
		return ret;
	    }
	}
	return NULL;
    }

    InfoType IFixedInfo::opNorm (Word op, syntax::Expression right) {
	if (this-> isSame (right-> info-> type)) {
	    auto ret = this-> clone ();
	    //TODO
	    return ret;
	} else if (auto ot = right-> info-> type-> to<IFixedInfo> ()) {
	    if (this-> isSigned () && ot-> isSigned ()) {
		if (this-> isSup (ot)) {
		    auto ret = this-> clone ();
		    //TODO
		    return ret;
		} else {
		    auto ret = ot-> clone ();
		    //TODO
		    return ret;
		}		    
	    } else if (!this-> isSigned () && !ot-> isSigned ()) {
		if (this-> isSup (ot)) {
		    auto ret = this-> clone ();
		    return ret;
		} else {
		    auto ret = ot-> clone ();
		    return ret;
		}
	    }
	}
	return NULL;
    }

    InfoType IFixedInfo::Init () {
	auto ret = new IFixedInfo (true, this-> _type);
	//TODO
	return ret;	
    }

    InfoType IFixedInfo::Max () {
	auto ret = new IFixedInfo (true, this-> _type);
	//TODO
	return ret;	
    }

    InfoType IFixedInfo::Min () {
	auto ret = new IFixedInfo (true, this-> _type);
	//TODO
	return ret;	
    }

    InfoType IFixedInfo::SizeOf () {
	auto ret = new IFixedInfo (true, FixedConst::UBYTE);
	//TODO
	return ret;	
    }

    InfoType IFixedInfo::StringOf () {
	auto str = new IStringInfo (true);
	return str;
    }	

    bool IFixedInfo::isSigned () {
	return syntax::isSigned (this-> _type);
    }
    
    bool IFixedInfo::isSup (FixedInfo fx) {
	return this-> _type > fx-> _type;
    }
    

}
