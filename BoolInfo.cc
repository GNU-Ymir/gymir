#include <ymir/semantic/types/_.hh>

namespace semantic {

    IBoolInfo::IBoolInfo (bool isConst) :
	IInfoType (isConst)
    {}
    
    bool IBoolInfo::isSame (InfoType other) {
	return other-> to<IBoolInfo> () != NULL;
    }
	
    InfoType IBoolInfo::BinaryOp (Word op, syntax::Expression right) {
	if (op == Token::EQUAL) return Affect (right);
	if (op == Token::DAND) return opNorm (op, right);
	if (op == Token::DPIPE) return opNorm (op, right);
	if (op == Token::NOT_EQUAL) return opNorm (op, right);
	if (op == Token::DEQUAL) return opNorm (op, right);
	return NULL;
    }

    InfoType IBoolInfo::BinaryOpRight (Word op, syntax::Expression left) {
	if (op == Token::EQUAL) return AffectRight (left);
	return NULL;
    }

    InfoType IBoolInfo::UnaryOp (Word) {
	if (op == Token::NOT) {
	    auto ret = new IBoolInfo (true);
	    //ret-> lintInstS.push_back (BoolUtils::InstXor);
	    //if (this-> value) ret-> value = this-> value-> UnaryOp (op);
	    return ret;
	} else if (op == Token::AND) return Ptr ();
	return NULL;
    }

    std::string IBoolInfo::innerTypeString () {
	return std::string ("bool");
    }

    std::string IBoolInfo::simpleTypeString () {
	return std::string ("b");
    }

    InfoType IBoolInfo::DotOp (syntax::Var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token.getStr () == "init") return Init ();
	if (var-> token.getStr () == "sizeof") return SizeOf ();
	if (var-> token.getStr () == "typeid") return StringOf ();
    }

    InfoType IBoolInfo::CastOp (InfoType) {
	//TODO
	return NULL;
    }

    InfoType IBoolInfo::CompOp (InfoType) {
	if (other-> is<IBoolInfo> () || other-> is<IUndefInfo> ()) {
	    auto bl = new IBoolInfo (this-> isConst ());
	    //bl-> lintInst = BoolUtils::InstAffect;
	    return bl;
	} else if (auto en = other-> to<IEnumInfo> ()) {
	    return this-> CompOp (en-> content);
	}
	return NULL;
    }

    InfoType IBoolInfo::clone () {
	return new IBoolInfo (this-> isConst ());
    }
		
    InfoType IBoolInfo::Ptr () {
	auto ptr = new IPtrInfo (this-> isConst (), this-> clone ());
	//ptr-> lintInstS.push_back (BoolUtils::InstAddr);
	return ptr;
    }

    InfoType IBoolInfo::Affect (syntax::Expression right) {
	if (right-> info-> type-> is<IBoolInfo> ()) {
	    auto b = new IBoolInfo (this-> isConst ());
	    //b-> lintInst = BoolUtils::InstAffect;
	    return b;
	}
	return NULL;
    }

    InfoType IBoolInfo::AffectRight (syntax::Expression left) {
	if (left-> info-> type-> is<IUndefInfo> ()) {
	    auto b = new IBoolInfo (this-> isConst ());
	    //b-> lintInst = BoolUtils::InstAffect;
	    return b;
	}
	return NULL;
    }

    InfoType IBoolInfo::opNorm (Word op, syntax::Expression right) {
	if (right-> info-> type-> is<IBoolInfo> ()) {
	    auto b = new IBoolInfo (true);
	    //TODO
	    return b;
	}
	return NULL;
    }

    InfoType IBoolInfo::Init () {
	//TODO
	return new IBoolInfo (true);
    }

    InfoType IBoolInfo::SizeOf () {
	//TODO
	return new IFixedInfo (true, FixedConst::UBYTE);
    }

    InfoType IBoolInfo::StringOf () {
	//TODO
	return new IStringInfo (true);
    }
    
    const char* IBoolInfo::getId () {
	return IBoolInfo::id ();
    }

    
}
