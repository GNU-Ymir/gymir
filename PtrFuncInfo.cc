#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/utils/PtrUtils.hh>

namespace semantic {
    
    IPtrFuncInfo::IPtrFuncInfo (bool isConst)
	: IInfoType (isConst)
    {}

    bool IPtrFuncInfo::isSame (InfoType other) {
	if (auto ptr = other-> to <IPtrFuncInfo> ()) {
	    if (!this-> ret-> isSame (ptr-> ret)) return false;
	    if (this-> params.size () != ptr-> params.size ())
		return false;
	    for (auto it : Ymir::r (0, this-> params.size ())) {
		if (!ptr-> params [it]-> isSame (this-> params [it]))
		    return false;
	    }
	    return true;
	}
	return false;
    }
    
    InfoType IPtrFuncInfo::BinaryOp (Word token, syntax::Expression right) {
	return NULL;
    }

    InfoType IPtrFuncInfo::BinaryOpRight (Word token, syntax::Expression left) {
	if (token == Token::EQUAL) return AffectRight (left);
	return NULL;
    }	

    ApplicationScore IPtrFuncInfo::CallOp (Word token, syntax::ParamList params) {
	return NULL;
    }

    InfoType IPtrFuncInfo::AffectRight (syntax::Expression left) {
	if (left-> info-> type-> is <IUndefInfo> ()) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = &PtrUtils::InstAffect;
	    return ret;
	}
	return NULL;
    }
    
    InfoType IPtrFuncInfo::CompOp (InfoType other) {
	if (other-> isSame (this) || other-> is <IUndefInfo> ()) {
	    auto ptr = this-> clone ();
	    ptr-> binopFoo = &PtrUtils::InstAffect;
	    return ptr;
	}
	return NULL;
    }

    InfoType IPtrFuncInfo::getTemplate (ulong i) {
	if (i < this-> params.size ()) return this-> params [i];
	else if (i == this-> params.size ())
	    return this-> ret;
	return NULL;
    }
    
    InfoType IPtrFuncInfo::onClone () {
	auto aux = new (Z0) IPtrFuncInfo (this-> isConst ());
	for (auto it : this-> params) {
	    aux-> params.push_back (it-> clone ());
	}
	aux-> ret = this-> ret-> clone ();
	aux-> score = this-> score;
	return aux;
    }

    InfoType IPtrFuncInfo::DotOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "typeid") {
	    return StringOf ();
	} else if (var-> token == "paramTuple") return NULL;
	else if (var-> token == "retType") return NULL;
	return NULL;
    }
 	
    std::string IPtrFuncInfo::innerTypeString () {
	Ymir::OutBuffer buf ("fn(");
	for (auto it : Ymir::r (0, this-> params.size ())) {
	    buf.write (this-> params [it]-> typeString ());
	    if (it < (int) this-> params.size () - 1)
		buf.write (",");
	}
	buf.write (")->", this-> ret-> typeString ());
	return buf.str ();
    }

    std::string IPtrFuncInfo::innerSimpleTypeString () {
	Ymir::OutBuffer buf ("PF");
	for (auto it : Ymir::r (0, this-> params.size ())) {
	    buf.write (this-> params [it]-> simpleTypeString ());
	}
	buf.write ("Z", this-> ret-> typeString ());
	return buf.str ();
    }

    InfoType& IPtrFuncInfo::getType () {
	return this-> ret;
    }

    std::vector <InfoType>& IPtrFuncInfo::getParams () {
	return this-> params;
    }

    ApplicationScore& IPtrFuncInfo::getScore () {
	return this-> score;
    }
    
    Ymir::Tree IPtrFuncInfo::toGeneric () {
	return build_pointer_type (
	    void_type_node
	);
    }

    const char * IPtrFuncInfo::getId () {
	return IPtrFuncInfo::id ();
    }
    
}
