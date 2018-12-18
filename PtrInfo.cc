#include <ymir/semantic/types/_.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/semantic/utils/PtrUtils.hh>
#include <ymir/semantic/tree/_.hh>
#include <ymir/ast/TreeExpression.hh>
#include <ymir/semantic/utils/FixedUtils.hh>
#include <ymir/semantic/value/FixedValue.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/semantic/object/AggregateInfo.hh>

namespace semantic {

    namespace PtrUtils {
	using namespace syntax;
	
	template <typename T>
	T getAndRemoveBack (std::list <T> &list) {
	    if (list.size () != 0) {
		auto last = list.back ();	    
		list.pop_back ();
		return last;
	    } else {
		return NULL;
	    }
	}

	Ymir::Tree InstAffect (Word locus, InfoType type, Expression left, Expression right) {
	    return FixedUtils::InstAffect (locus, type, left, right);
	}
	
	Ymir::Tree InstCast (Word locus, InfoType, Expression elem, Expression typeExpr) {
	    auto type = typeExpr-> info-> type ()-> toGeneric ();
	    auto lexp = elem-> toGeneric ();
	    return fold_convert_loc (locus.getLocus (), type.getTree (), lexp.getTree ());
	}

	Ymir::Tree InstPlus (Word locus, InfoType, Expression left, Expression right) {
	    auto ltree = left-> toGeneric ();
	    Ymir::Tree rtree = convert (long_unsigned_type_node, right-> toGeneric ().getTree ());
	    tree_code code = POINTER_PLUS_EXPR;
	    return Ymir::buildTree (
		code, locus.getLocus (), ltree.getType (), ltree, rtree
	    );
	}
	
	Ymir::Tree InstPPlus (Word locus, InfoType, Expression left) {
	    auto ltree = left-> toGeneric ();
	    auto rtree = build_int_cst_type (long_unsigned_type_node, 1);
	    tree_code code = POINTER_PLUS_EXPR;
	    return Ymir::buildTree (
		MODIFY_EXPR, locus.getLocus (), ltree.getType (), ltree,
		Ymir::buildTree (
		    code, locus.getLocus (), ltree.getType (), ltree, rtree
		)
	    );
	}

	Ymir::Tree InstUnrefUn (Word locus, InfoType t, Expression left) {
	    auto type = t-> cloneOnExitWithInfo ();
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);
	    
	    auto inner = left-> info-> type ()-> to<IPtrInfo> ()-> content ()-> toGeneric ();
	    
	    auto leftExp = left-> toGeneric ();
	    leftExp = getPointerUnref (locus.getLocus (), leftExp, inner, 0);

	    if (type-> unopFoo) {
		return type-> buildUnaryOp (
		    locus,
		    type,
		    new (Z0)  ITreeExpression (left-> token, left-> info-> type (), leftExp)
		);
	    } else if (type-> binopFoo) {
		return type-> buildBinaryOp (
		    locus,
		    type,
		    new (Z0)  ITreeExpression (left-> token, left-> info-> type (), leftExp),
		    new (Z0)  ITreeExpression (locus, type, Ymir::Tree ())
		);
	    } else {
		return leftExp;
	    }
	}
	
	Ymir::Tree InstUnrefBin (Word locus, InfoType t, Expression left, Expression right) {
	    auto type = t-> cloneOnExitWithInfo ();
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);

	    auto innerType = left-> info-> type ()-> to<IPtrInfo> ()-> content ();
	    auto inner = innerType-> toGeneric ();	    
	    auto leftExp = left-> toGeneric ();
	    leftExp = getPointerUnref (locus.getLocus (), leftExp, inner, 0);
	    
	    if (type-> binopFoo) {
		return type-> buildBinaryOp (
		    locus,
		    type,
		    new (Z0)  ITreeExpression (left-> token, innerType, leftExp),
		    right
		);
	    } else if (type-> unopFoo) {
		return type-> buildUnaryOp (
		    locus,
		    type,
		    new (Z0)  ITreeExpression (left-> token, innerType, leftExp)
		);
	    }
	    return leftExp;
	}

	Ymir::Tree InstUnrefMult (Word locus, InfoType t, Expression left, Expression right, ApplicationScore score) {
	    auto type = t-> cloneOnExitWithInfo ();
	    type-> binopFoo = getAndRemoveBack (type-> nextBinop);
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);
	    type-> multFoo = getAndRemoveBack (type-> nextMult);

	    auto innerType = left-> info-> type ()-> to<IPtrInfo> ()-> content ();
	    auto inner = innerType-> toGeneric ();	    
	    auto leftExp = left-> toGeneric ();
	    leftExp = getPointerUnref (locus.getLocus (), leftExp, inner, 0);
	    
	    if (type-> multFoo) {
		return type-> buildMultOp (
		    locus,
		    type,
		    new (Z0)  ITreeExpression (left-> token, innerType, leftExp),
		    right,
		    score
		);
	    }
	    
	    return leftExp;
	}

	
	Ymir::Tree InstPlusRight (Word locus, InfoType, Expression left, Expression right) {
	    auto ltree = right-> toGeneric ();
	    Ymir::Tree rtree = convert (long_unsigned_type_node, left-> toGeneric ().getTree ());
	    tree_code code = POINTER_PLUS_EXPR;
	    return Ymir::buildTree (
		code, locus.getLocus (), ltree.getType (), ltree, rtree
	    );
	}
	
	Ymir::Tree InstSub (Word locus, InfoType, Expression left, Expression right) {
	    auto ltree = left-> toGeneric ();
	    Ymir::Tree rtree = Ymir::buildTree (
		NEGATE_EXPR, locus.getLocus (), long_unsigned_type_node,
		convert (long_unsigned_type_node, right-> toGeneric ().getTree ())
	    );
	    
	    tree_code code = POINTER_PLUS_EXPR;
	    return Ymir::buildTree (
		code, locus.getLocus (), ltree.getType (), ltree, rtree
	    );
	}
	
	Ymir::Tree InstUnref (Word locus, InfoType type, Expression elem) {
	    auto inner = type-> toGeneric ();
	    return Ymir::getPointerUnref (locus.getLocus (), elem-> toGeneric (), inner, 0);
	}
	

	Ymir::Tree InstUnrefTyped (Word locus, InfoType type, Expression elem, Expression) {
	    auto inner = type-> toGeneric ();
	    return Ymir::getPointerUnref (locus.getLocus (), elem-> toGeneric (), inner, 0);
	}

	Ymir::Tree InstTypeInfo (Word, InfoType, Expression elem) {
	    return elem-> info-> type ()-> genericTypeInfo ();
	}
	

    }
    
    IPtrInfo::IPtrInfo (bool isConst) :
	IInfoType (isConst),
	_content (NULL)
    {
	Ymir::Error::assert ("WH!!");
    }
    
    IPtrInfo::IPtrInfo (bool isConst, InfoType type) :
	IInfoType (isConst),
	_content (type) 
    {
	//this-> isText () = isConst;
    }    
   
    InfoType IPtrInfo::create (Word tok, const std::vector<syntax::Expression> & tmps) {
	if (tmps.size () != 1 || tmps [0] == NULL || !tmps [0]-> isType ()) {
	    if (tmps.size () == 1)
		println (tmps [0] && tmps [0]-> isType ());
	    
	    Ymir::Error::takeATypeAsTemplate (tok);
	    return NULL;	    
	}	    	    

	if (auto ot = tmps [0]-> info-> type ()-> to<IStructCstInfo> ()) {
	    auto type = ot-> TempOp ({});
	    if (type == NULL) return NULL;
	    return new (Z0) IPtrInfo (false, type);
	} else if (auto ag = tmps [0]-> info-> type ()-> to <IAggregateCstInfo> ()) {
	    auto type = ag-> TempOp ({});
	    if (type == NULL) return NULL;
	    return new (Z0) IPtrInfo (false, type);
	} else {
	    return new (Z0) IPtrInfo (false, tmps [0]-> info-> type ());
	}
    }

    const char * IPtrInfo::getId () {
	return IPtrInfo::id ();
    }

    InfoType IPtrInfo::BinaryOp (Word token, syntax::Expression right) {
	if (token == Token::EQUAL) return Affect (right);
	else if (token == Token::PLUS) return Plus (right);
	else if (token == Token::MINUS) return Sub (right);
	else if (token == Keys::IS) return Is (right);
	else if (token == Keys::NOT_IS) return NotIs (right);
	return NULL;
    }

    InfoType IPtrInfo::BinaryOpRight (Word token, syntax::Expression left) {
	if (token == Token::EQUAL) return AffectRight (left);
	else if (token == Token::PLUS) return PlusRight (left);
	return NULL;
    }

    InfoType IPtrInfo::UnaryOp (Word op) {
	if (op == Token::STAR) return Unref (op);
	else if (op == Token::AND && this-> isLvalue ()) return toPtr ();
	else if (op == Token::DPLUS) {
	    auto ret = this-> cloneConst ();
	    ret-> unopFoo = PtrUtils::InstPPlus;
	    return ret;
	}	
	return NULL;
    }

    InfoType IPtrInfo::Affect (syntax::Expression right) {
	auto type = right-> info-> type ()-> to<IPtrInfo> ();
	if (type != NULL && type-> _content-> isSame (this-> _content)) {
	    auto ret = new (Z0)  IPtrInfo (false, this-> _content-> clone ());
	    ret-> binopFoo = &PtrUtils::InstAffect;
	    return ret;
	} else if (type && this-> _content-> is<IVoidInfo> ()) {
	    this-> _content = type-> _content-> clone ();
	    auto ret = new (Z0)  IPtrInfo (false, this-> _content-> clone ());
	    ret-> binopFoo = &PtrUtils::InstAffect;
	    return ret;
	} else if (type && type-> _content-> is <IVoidInfo> ()) {
	    auto ret = new (Z0)  IPtrInfo (false, type-> _content-> clone ());
	    ret-> binopFoo = &PtrUtils::InstAffect;
	    return ret;
	} else if (right-> info-> type ()-> is <INullInfo> ()) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = &PtrUtils::InstAffect;
	    return ret;
	}
	return NULL;
    }

    InfoType IPtrInfo::AffectRight (syntax::Expression left) {
	if (left-> info-> type ()-> is<IUndefInfo> ()) {
	    auto ret = new (Z0)  IPtrInfo (false, this-> _content-> clone ());
	    ret-> binopFoo = &PtrUtils::InstAffect;
	    return ret;
	}
	return NULL;
    }

    InfoType IPtrInfo::Plus (syntax::Expression right) {
	if (right-> info-> type ()-> is<IFixedInfo> ()) {
	    auto ptr = new (Z0)  IPtrInfo (this-> isConst (), this-> _content-> clone ());
	    ptr-> binopFoo = &PtrUtils::InstPlus;
	    return ptr;
	}
	return NULL;
    }
    
    InfoType IPtrInfo::Sub (syntax::Expression right) {
	if (right-> info-> type ()-> is<IFixedInfo> ()) {
	    auto ptr = new (Z0)  IPtrInfo (this-> isConst (), this-> _content-> clone ());
	    ptr-> binopFoo = &PtrUtils::InstSub;
	    return ptr;
	}
	return NULL;
    }

    InfoType IPtrInfo::PlusRight (syntax::Expression right) {
	if (right-> info-> type ()-> is<IFixedInfo> ()) {
	    auto ptr = new (Z0)  IPtrInfo (this-> isConst (), this-> _content-> clone ());
	    ptr-> binopFoo = &PtrUtils::InstPlusRight;
	    return ptr;
	}
	return NULL;
    }
    

    InfoType IPtrInfo::Is (syntax::Expression right) {
	if (right-> info-> type ()-> is <IPtrInfo> ()) {
	    auto ret = new (Z0)  IBoolInfo (true);
	    ret-> binopFoo = &FixedUtils::InstTest;
	    return ret;
	} else if (right-> info-> type ()-> is <INullInfo> ()) {
	    auto ret = new (Z0)  IBoolInfo (true);
	    ret-> binopFoo = &FixedUtils::InstTest;
	    return ret;
	}
	return NULL;
    }
    
    InfoType IPtrInfo::NotIs (syntax::Expression right) {
	if (right-> info-> type ()-> is <IPtrInfo> ()) {
	    auto ret = new (Z0)  IBoolInfo (true);
	    ret-> binopFoo = &FixedUtils::InstTest;
	    return ret;
	} else if (right-> info-> type ()-> is <INullInfo> ()) {
	    auto ret = new (Z0)  IBoolInfo (true);
	    ret-> binopFoo = &FixedUtils::InstTest;
	    return ret;
	}
	return NULL;
    }
    
    InfoType IPtrInfo::Unref (Word & op) {	
	if (this-> _content-> is<IUndefInfo> ()) return NULL;
	else if (this-> _content-> is <IVoidInfo> ()) return NULL;
	else {
	    if (Table::instance ().hasCurrentContext (Keys::SAFE)) {
		Ymir::Error::unrefInSafe (op);
	    }
	    auto ret = this-> _content-> clone ();
	    if (this-> isConst ()) ret-> isConst (this-> isConst ());
	    ret-> unopFoo = &PtrUtils::InstUnref;
	    return ret;
	}
    }

    InfoType IPtrInfo::toPtr () {
	auto ret = new (Z0)  IPtrInfo (this-> isConst (), this-> clone ());
	ret-> binopFoo = &FixedUtils::InstAddr;
	return ret;
    }

    InfoType IPtrInfo::DotOp (syntax::Var var) {
	if (var-> isType ()) {
	    auto type = var-> asType ();
	    if (type) {
		auto ret = type-> info-> type ();
		ret-> unopFoo = &PtrUtils::InstUnref;
		ret-> binopFoo = &PtrUtils::InstUnrefTyped;
		return ret;
	    }
	    return NULL;
	} else {
	    auto aux = this-> _content-> DotOp (var);
	    if (aux != NULL) return addUnref (aux);
	    return NULL;
	}
	return NULL;
    }
    
    InfoType IPtrInfo::DColonOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "init") {
	    auto type = this-> clone ();
	    type-> value () = new (Z0) IFixedValue (FixedConst::ULONG, 0, 0);
	    return type;
	} else if (var-> token == "typeid") {
	    return StringOf ();
	} else if (var-> token == "sizeof") {
	    auto ret = new (Z0)  IFixedInfo (true, FixedConst::UINT);
	    ret-> unopFoo = FixedUtils::InstSizeOf;
	    return ret;	
	} else if (var-> token == "typeinfo") {
	    auto ret = this-> _content-> DColonOp (var);
	    if (ret != NULL) {
	        ret-> nextUnop.push_back (ret-> unopFoo);
	        ret-> unopFoo = PtrUtils::InstTypeInfo;
	    }
	    return ret;
	}
	return NULL;
    }
    
    InfoType& IPtrInfo::content () {
	return this-> _content;
    }

    InfoType IPtrInfo::CastOp (InfoType other) {
	auto type = other-> to<IPtrInfo> ();
	if (type && type-> _content-> isSame (this-> _content)) {
	    return this;
	} else if (type) {
	    auto ptr = new (Z0)  IPtrInfo (this-> isConst (), type-> _content-> clone ());
	    ptr-> binopFoo = &PtrUtils::InstCast;
	    return ptr;
	    // }  else if (auto tu = other-> to <ITupleInfo> ()) {
	} else if (auto ul = other-> to <IFixedInfo> ()) {
	    if (ul-> type () == FixedConst::ULONG) {
		auto ot = ul-> cloneOnExit ();
		ot-> binopFoo = &PtrUtils::InstCast;
		return ot;
	    }
	}
	return NULL;
    }

    InfoType IPtrInfo::CompOp (InfoType other) {
	if (other-> isSame (this) || other-> is<IUndefInfo> ()) {
	    auto ptr = this-> clone ();
	    ptr-> binopFoo = &PtrUtils::InstCast;
	    return ptr;
	} else if (auto ref = other-> to<IRefInfo> ()) {
	    if (this-> isLvalue () && this-> isSame (ref-> content ())) {
		auto aux = new (Z0)  IRefInfo (this-> isConst (), this-> clone ());
		aux-> binopFoo = &FixedUtils::InstAddr;
		return aux;
	    }
	} 
	return NULL;
    }
    
    std::string IPtrInfo::innerTypeString () {
	return "p!(" + this-> _content-> typeString () + ")";
    }

    std::string IPtrInfo::innerSimpleTypeString () {
	return "P" + Mangler::mangle_type (this-> _content, this-> _content-> simpleTypeString ());
    }

    InfoType IPtrInfo::onClone () {
	return new (Z0) IPtrInfo (this-> isConst (), this-> _content-> clone ());
    }
    
    InfoType IPtrInfo::ConstVerif (InfoType other) {
	if (auto ot = other-> to<IPtrInfo> ()) {
	    if (other-> isConst ()) return this;
	    else if (!this-> _content-> ConstVerif (ot-> _content)) return NULL;
	    else if (this-> _content-> isConst () && !ot-> _content-> isConst ())
		return NULL;
	    return this;
	}
	return NULL;
    }

    bool IPtrInfo::passingConst (InfoType other) {
	if (IInfoType::passingConst (other)) return true;
	else if (auto type = other-> to <IPtrInfo> ()) {
	    return this-> _content-> passingConst (type-> _content);
	}
	return false;
    }

    bool IPtrInfo::needKeepConst () {
	return this-> isConst ();
    }
    
    bool IPtrInfo::isConst () {
	return IInfoType::isConst ();
    }
    
    void IPtrInfo::isConst (bool isConst) {
	IInfoType::isConst (isConst);
    }
    
    bool IPtrInfo::isSame (InfoType other) {
	if (auto ot = other-> to<IPtrInfo> ()) {
	    return ot-> _content-> isSame (this-> _content); 
	}
	return NULL;
    }

    InfoType IPtrInfo::addUnref (InfoType elem) {
	bool binop = false, unop = false, mult = false;
	if (elem-> binopFoo) {
	    elem-> nextBinop.push_back (elem-> binopFoo);
	    binop = true;
	}
	if (elem-> unopFoo) {
	    elem-> nextUnop.push_back (elem-> unopFoo);
	    unop = true;
	}

	if (elem-> multFoo) {
	    elem-> nextMult.push_back (elem-> multFoo);
	    mult = true;
	}
	
	if (binop) elem-> binopFoo = &PtrUtils::InstUnrefBin;
	if (unop) elem-> unopFoo = &PtrUtils::InstUnrefUn;
	if (mult) elem-> multFoo = &PtrUtils::InstUnrefMult;
	return elem;
    }
    

    Ymir::Tree IPtrInfo::toGeneric () {
	if (this-> _content-> to<IStructInfo> () ||
	    this-> _content-> to <IAggregateInfo> ()
	) {
	    return build_pointer_type (
		void_type_node
	    );
	} else {
	    return build_pointer_type (
		this-> _content-> toGeneric ().getTree ()
	    );
	}
    }

    Ymir::Tree IPtrInfo::genericTypeInfo () {
	auto inner = this-> content ()-> genericTypeInfo ();

	auto type = Table::instance ().getTypeInfoType ()-> TempOp ({});
	auto typeTree = type-> toGeneric ();
	vec <constructor_elt, va_gc> * elms = NULL;
       	    
	auto ptr_info_type = Table::instance ().getTypeInfoType (Ymir::Runtime::PTR_INFO)-> TempOp ({})-> to <IAggregateInfo> ();
	auto vtable = ptr_info_type-> getVtable ();
	    
	CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (typeTree, Keys::VTABLE_FIELD).getTree (), Ymir::getAddr (vtable).getTree ());	   
	CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (typeTree, Ymir::Runtime::VTABLE_FIELD_TYPEINFO).getTree (), build_int_cst_type (long_unsigned_type_node, 0));

	CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (typeTree, Ymir::Runtime::LEN_FIELD_TYPEINFO).getTree (), build_int_cst_type (long_unsigned_type_node, 0));
	CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (typeTree, Ymir::Runtime::C_O_A_TYPEINFO).getTree (), getAddr (inner).getTree ());
	
	auto name = Ymir::Runtime::TYPE_INFO_MODULE + "." + this-> innerSimpleTypeString () + Ymir::Runtime::TYPE_INFO_SUFFIX;
	auto glob = Ymir::declareGlobalWeak (name, typeTree, build_constructor (typeTree.getTree (), elms));

	return glob;
    }
    
    Ymir::Tree IPtrInfo::genericConstructor () {
	return build_int_cst_type (long_unsigned_type_node, 0);
    }
    
    InfoType IPtrInfo::getTemplate (ulong nb) {
	if (nb == 0) return this-> _content;
	return NULL;
    }


}
