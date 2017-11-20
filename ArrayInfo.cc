#include <ymir/semantic/types/_.hh>

namespace semantic {

    IArrayInfo::IArrayInfo (bool isConst, InfoType content) :
	IInfoType (isConst),
	content (content)
    {
	if (this-> content)
	    this-> content-> isConst () = this-> isConst ();
    }

    
    InfoType IArrayInfo::content () {
	return this-> content;
    }

    bool IArrayInfo::isSame (InfoType other) {
	auto arr = other-> to<ArrayInfo> ();
	if (arr == NULL) return NULL;
	if (this-> content == arr-> content) return true;
	return this-> content-> isSame (arr-> content);
    }

    InfoType IArrayInfo::Is (Expression right) {
	if (auto ptr = right-> info-> type-> to<INullInfo> ()) {
	    auto ret = new IBoolInfo (true);
	    //ret-> lintInst = ArrayUtils::InstIsNull;
	    return ret;
	} else if (this-> isSame (right-> info-> type)) {
	    auto ret = new IBoolInfo (true);
	    //ret-> lintInst = ArrayUtils::InstIs;
	    return ret;
	}
	return NULL;
    }

    InfoType IArrayInfo::NotIs (Expression right) {
	if (auto ptr = right-> info-> type-> to<INullInfo> ()) {
	    auto ret = new IBoolInfo (true);
	    //ret-> lintInst = ArrayUtils::InstNotIsNull;
	    return ret;
	} else if (this-> isSame (right-> info-> type)) {
	    auto ret = new IBoolInfo (true);
	    //ret-> lintInst = ArrayUtils::InstNotIs;
	    return ret;
	}
	return NULL;
    }

    InfoType IArrayInfo::Affect (Expression right) {
	auto type = right-> info-> type-> to<IArrayInfo> ();
	if (type && type-> content-> isSame (this-> content)) {
	    auto ret = this-> clone ();
	    //ret-> lintInst = ArrayUtils::InstAffect;
	    return ret;
	} else if (type && this-> content-> is<IVoidInfo> ()) {
	    this-> content = type-> content-> clone ();
	    auto ret = this-> clone ();
	    //ret-> lintInst = ArrayUtils::InstAffect;
	    return ret;
	} else if (left-> info-> type-> is<INullInfo> ()) {
	    auto ret = this-> clone ();
	    //ret-> lintInst = ArrayUtils::InstAffectNull;
	    return ret;
	}
	return NULL;
    }
    
    
    InfoType IArrayInfo::BinaryOp (Word token, syntax::Expression right) {
	if (token == Token::EQUAL) return Affect (right);
	if (token == Keys::IS) return Is (right);
	if (token == Keys::NOT_IS) return NotIs (right);
	return NULL;
    }

    InfoType IArrayInfo::BinaryOpRight (Word token, syntax::Expression left) {
	if (token == Token::EQUAL) return AffectRight (left);
	return NULL;
    }

    InfoType IArrayInfo::ApplyOp (std::vector <syntax::Var> vars) {
	if (vars.size () != 1) return NULL;
	if (this-> isConst ()) {
	    vars [0]-> info-> type = this-> content-> clone ();
	} else {
	    vars [0]-> info-> type = new IRefInfo (false, this-> content-> clone ());
	}
	auto ret = this-> clone ();
	// TODO
	ret-> isConst ()= this-> isConst ();
	return ret;
    }
	
    InfoType IArrayInfo::AccessOp (Word, syntax::ParamList params) {
	if (params-> getParams ().size () == 1) {
	    return Access (params-> getParams () [0]);
	}
	return NULL;
    }

    InfoType IArrayInfo::DotOp (syntax::Var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token.getStr () == "len") return Length ();
	if (var-> token.getStr () == "typeid") return TypeId ();
	if (var-> token.getStr () == "ptr") return Ptr ();
	if (var-> token.getStr () == "tupleof") return TupleOf ();
	return NULL;
    }

    InfoType IArrayInfo::Ptr () {
	auto ret = new IPtrInfo (this-> isConst (), this-> content-> clone ());
	//ret-> lintInst = ArrayUtils::InstPtr;
	return ret;
    }

    InfoType IArrayInfo::Length () {
	if (this-> content-> is<IVoidInfo> ()) return NULL;
	auto elem = new IFixedInfo (true, FixedConst::ULONG);
	//elem-> lintInst = ArrayUtils::InstLength;
	return elem;
    }

    InfoType IArrayInfo::TypeId () {
	auto str = new IStringInfo (true);
	//str-> value = new IStringValue (this-> typeString ());
	return str;
    }

    InfoType IArrayInfo::TupleOf () {
	auto t = new ITupleInfo (this-> isConst ());
	t-> params = {new IFixedInfo (this-> isConst (), FixedConst::ULONG),
		      new IPtrInfo (this-> isConst (), this-> content-> clone ())
	}
	//t-> lintInst = ArrayUtils::InstCastTuple;
	return t;
    }    

    InfoType IArrayInfo::Access (syntax::Expression expr) {
	if (auto ot = expr-> info-> type-> to<IFixedInfo> ()) {
	    auto ch = this-> content-> clone ();
	    //TODO
	    return ch;
	}
	return NULL;
    }
    
    InfoType IArrayInfo::clone () {
	auto ret = new IArrayInfo (this-> isConst (), this-> content-> clone ());
	//ret-> value = this-> value;
	return ret;
    }

    InfoType IArrayInfo::CastOp (InfoType other) {
	auto type = other-> to<IArrayInfo> ();
	if (type && type-> content-> isSame (this-> content)) {
	    return this;
	} else if (other-> is<IStringInfo> () && this-> content-> is<ICharInfo> ()) {
	    auto other_ = new IStringInfo (this-> isConst ());
	    //other_-> lintInstS.push_back (ArrayUtils::InstCastString);
	    return other_;
	}
	return NULL;
    }

    InfoType IArrayInfo::CompOp (InfoType other) {
	auto type = other-> to<IArrayInfo> ();
	if ((type && type-> content-> isSame (this-> content)) ||
	    other-> is<IUndefInfo> ()) {
	    auto ret = this-> clone ();
	    //ret-> lintInst = ArrayUtils::InstAffectRight;
	    return ret;
	} else if (type && this-> content-> is<IVoidInfo> ()) {
	    auto ret = this-> clone ();
	    // ret-> leftTreatment = ArrayUtils::InstCastFromNull;
	    // ret-> lintInst = ArrayUtils::InstAffectRight;
	    return ret;
	} else if (auto ref = other-> to<IRefInfo> ()) {
	    if (auto arr = ref-> content-> to<IArrayInfo> ()) {
		if (arr-> content-> isSame (this-> content) && !this-> isConst ()) {
		    auto aux = new IRefInfo (this-> clone ());
		    aux-> lintInstS.push_back (&ArrayUtils::InstAddr);
		    return aux;
		}
	    }
	} else if (auto en = other-> to<IEnumInfo> ()) {
	    return this-> CompOp (en-> content);
	} else if (other-> is<INullInfo> ()) {
	    return this-> clone ();	    
	}	    
	return NULL;
    }

    InfoType IArrayInfo::ConstVerif (InfoType) {
	if (this-> isConst () && !other-> isConst ()) return NULL;
	else if (!this-> isConst ()&& other-> isConst ()) {
	    this-> isConst ()= false;
	}
	return this;
    }

    std::string IArrayInfo::innerTypeString () {
	return std::string ("[") + this-> content-> innerTypeString () + "]";
    }

    std::string IArrayInfo::simpleTypeString () {
	if (this-> isConst ()) return std::string ("cA") + this-> content-> simpleTypeString ();
	else return std::string ("A") + this-> content-> simpleTypeString ();
    }

    InfoType IArrayInfo::getTemplate (ulong i) {
	if (i == 0) return this-> content;
	else return NULL;
    }
    
    const char* IArrayInfo::getId () {
	return IArrayInfo::id ();
    }
    
}
