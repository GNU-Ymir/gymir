#include <ymir/semantic/types/_.hh>
#include <ymir/ast/ParamList.hh>
#include <ymir/ast/ConstArray.hh>
#include <ymir/ast/TreeExpression.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/semantic/pack/FinalFrame.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/semantic/utils/ArrayUtils.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/semantic/pack/InternalFunction.hh>
#include <ymir/semantic/value/FixedValue.hh>
#include <ymir/semantic/value/StringValue.hh>

namespace semantic {

    using namespace syntax;
    
    IArrayInfo::IArrayInfo (bool isConst, InfoType content) :
	IInfoType (isConst),
	_isStatic (false),
	_size (0),
	_content (content)
    {}
    
    InfoType IArrayInfo::content () {
	return this-> _content;
    }

    bool IArrayInfo::isSame (InfoType other) {
	auto arr = other-> to<IArrayInfo> ();
	if (arr == NULL) return NULL;
	if (this-> _isStatic != arr-> _isStatic || this-> _size != arr-> _size) return false;
	if (this-> _content == arr-> _content) return true;
	return this-> _content-> isSame (arr-> _content);
    }

    InfoType IArrayInfo::Affect (Expression right) {
	auto type = right-> info-> type-> to<IArrayInfo> ();
	if (type && type-> _content-> isSame (this-> _content)) {
	    if (type-> _isStatic != this-> _isStatic || type-> _size != this-> _size)
		return NULL;
	    
	    auto ret = type-> clone ();
	    ret-> isConst (this-> isConst ());
	    if (type-> _content-> ConstVerif (this-> _content) == NULL)
		return NULL;
	    
	    ret-> binopFoo = ArrayUtils::InstAffect;
	    return ret;
	} else if (type && this-> _content-> is<IVoidInfo> ()) {
	    if (this-> _isStatic) return NULL;
	    this-> _content = type-> _content-> clone ();
	    auto ret = this-> clone ();
	    ret-> binopFoo = ArrayUtils::InstAffectNull;
	    return ret;
	} else if (right-> info-> type-> is<INullInfo> ()) {
	    if (this-> _isStatic) return NULL;
	    auto ret = this-> clone ();
	    ret-> binopFoo = ArrayUtils::InstAffectNull;
	    return ret;
	}
	return NULL;
    }

    InfoType IArrayInfo::AffectRight (Expression left) {
	if (left-> info-> type-> is<IUndefInfo> ()) {
	    auto arr = this-> clone ();
	    arr-> isConst (false);
	    arr-> binopFoo = ArrayUtils::InstAffect;
	    return arr;
	}
	return NULL;
    }
    
    
    InfoType IArrayInfo::BinaryOp (Word token, syntax::Expression right) {
	if (token == Token::EQUAL) return Affect (right);
	if (token == Token::TILDE) {
	    auto aux = Concat (right);
	    if (aux && this-> value ()) {
		aux-> value () = this-> value ()-> BinaryOp (token, right-> info-> value ());
	    }
	    return aux;
	} else if (token == Token::TILDE_EQUAL) return ConcatAff (right);
	return NULL;
    }

    InfoType IArrayInfo::BinaryOpRight (Word token, syntax::Expression left) {
	if (token == Token::EQUAL) return AffectRight (left);
	return NULL;
    }

    InfoType IArrayInfo::ApplyOp (const std::vector<syntax::Var> & vars) {
	if (vars.size () != 1) return NULL;
	if (this-> isConst ()) {
	    vars [0]-> info-> type = this-> _content-> clone ()-> CompOp (vars [0]-> info-> type);
	} else {
	    InfoType ref = new (Z0) IRefInfo (false, this-> _content);
	    auto content = this-> _content-> clone ();
	    content-> value () = NULL;
	    content-> isConst (false);
	    vars [0]-> info-> type = content-> CompOp (ref);	    
	}
	if (vars [0]-> info-> type == NULL) return NULL;
	auto ret = this-> clone ();
	ret-> applyFoo = &ArrayUtils::InstApply;
	return ret;
    }

    ApplicationScore IArrayInfo::CallOp (Word token, syntax::ParamList params) {
	if (params-> getParams ().size () != 2 && !this-> _isStatic) 
	    return NULL;
	
	auto fst = params-> getParams ()[0];
	auto scd = params-> getParams ()[1];

	auto score = new (Z0) IApplicationScore (token);
	score-> treat.push_back (fst-> info-> type-> CompOp (new (Z0) IFixedInfo (true, FixedConst::ULONG)));
	if (!score-> treat.back ()) return NULL;

	score-> treat.push_back (scd-> info-> type-> CompOp (new (Z0) IPtrInfo (false, this-> _content)));
	if (!score-> treat.back ()) return NULL;
	if (scd-> info-> type-> isConst () && !this-> isConst ()) return NULL;	    
	
	auto res = new (Z0) IArrayInfo (false, this-> _content-> clone ());
	res-> multFoo = &ArrayUtils::InstCall;
	score-> dyn = true;
	score-> ret = res;
	return score;
    }
    
    InfoType IArrayInfo::AccessOp (Word, syntax::ParamList params, std::vector <InfoType> & treats) {
	if (params-> getParams ().size () == 1) {
	    return Access (params-> getParams () [0], treats [0]);
	}
	return NULL;
    }

    InfoType IArrayInfo::DotOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "len") return Length ();
	if (var-> token == "typeid") return StringOf ();
	if (var-> token == "ptr") return Ptr ();
	//if (var-> token == "tupleof") return TupleOf ();
	return NULL;
    }

    InfoType IArrayInfo::DColonOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "typeid") return StringOf ();
	if (var-> token == "init") return Init ();
	return NULL;
    }

    InfoType IArrayInfo::Init () {
	if (!this-> _isStatic) {
	    auto ret = this-> clone ();
	    ret-> unopFoo = ArrayUtils::InstInit;
	    return ret;
	}
	return NULL;
    }
    
    InfoType IArrayInfo::Ptr () {
	auto ret = new (Z0)  IPtrInfo (this-> isConst (), this-> _content-> clone ());
	ret-> binopFoo = ArrayUtils::InstPtr;
	return ret;
    }

    InfoType IArrayInfo::Length () {
	auto elem = new (Z0)  IFixedInfo (true, FixedConst::ULONG);
	elem-> binopFoo = ArrayUtils::InstLen;
	if (this-> _isStatic) {
	    elem-> value () = new (Z0) IFixedValue (FixedConst::ULONG, this-> _size, this-> _size);
	} else if (this-> value ()) {
	    if (auto str = this-> value ()-> to <IStringValue> ()) 
		elem-> value () = str-> getLen ();
	}
	
	return elem;
    }

    InfoType IArrayInfo::Concat (syntax::Expression right) {
	auto other = right-> info-> type;
	if (auto ot = other-> to<IArrayInfo> ()) {
	    if (ot-> _content-> isSame (this-> _content)) {
		auto i = this-> clone ()-> to <IArrayInfo> ();
		i-> isStatic (false, 0);
		i-> isConst (true);
		i-> binopFoo = &ArrayUtils::InstConcat;
		return i;
	    }
	}
	return NULL;
    }

    InfoType IArrayInfo::ConcatAff (syntax::Expression right) {
	if (this-> isConst ()) return NULL;
	if (right-> info-> type-> isSame (this)) {
	    auto i = this-> clone ();
	    i-> isConst (false);
	    i-> binopFoo = &ArrayUtils::InstConcatAff;
	    return i;
	}
	return NULL;
    }
    
    InfoType IArrayInfo::Access (syntax::Expression expr, InfoType& treat) {
	treat = expr-> info-> type-> CompOp (new (Z0)  IFixedInfo (true, FixedConst::LONG));
	if (treat == NULL) {
	    treat = expr-> info-> type-> CompOp (new (Z0)  IFixedInfo (true, FixedConst::ULONG));
	}
	
	if (treat) {
	    auto content = this-> _content-> clone ();
	    if (this-> isConst ())
		content-> isConst (true);
	    auto ch = new (Z0)  IArrayRefInfo (this-> isConst (), content);
	    ch-> binopFoo = &ArrayUtils::InstAccessInt;
	    if (this-> value ()) 
		ch-> value () = this-> value ()-> AccessOp (expr);
	    
	    return ch;
	}
	return NULL;
    }
    
    InfoType IArrayInfo::onClone () {
	auto ret = new (Z0)  IArrayInfo (this-> isConst (), this-> _content-> clone ());
	//ret-> value = this-> value;
	ret-> isStatic (this-> _isStatic, this-> _size);
	ret-> _content-> isConst (this-> _content-> isConst ());
	return ret;
    }

    InfoType IArrayInfo::CastOp (InfoType other) {
	auto type = other-> to<IArrayInfo> ();
	if (type && type-> _content-> isSame (this-> _content)) {
	    if (type-> _isStatic != this-> _isStatic) return NULL;
	    else if (this-> _isStatic && type-> _size > this-> _size) return NULL;
	    else if (this-> _isStatic) {
		auto ret = other-> clone ();
		ret-> binopFoo = ArrayUtils::InstToArray;		
		return ret;
	    }
	    return this;
	}
	return NULL;
    }

    InfoType IArrayInfo::CompOp (InfoType other) {
	auto type = other-> to<IArrayInfo> ();	
	if (type && type-> _content-> isSame (this-> _content)) {
	    ArrayInfo ret;
	    if (this-> _isStatic && !type-> _isStatic) {
		ret = type-> clone ()-> to <IArrayInfo> ();		
	    } else if (type-> _isStatic != this-> _isStatic || type-> _size != this-> _size)
		return NULL;
	    else { 	   
		ret = type-> clone ()-> to<IArrayInfo> ();
	    }
	    
	    ret-> isConst (this-> isConst ());
	    ret-> _content-> isConst (this-> _content-> isConst ());
	    ret-> binopFoo = ArrayUtils::InstToArray;
	    return ret;	    
	} else if (other-> is<IUndefInfo> ()) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = ArrayUtils::InstToArray;
	    //ret-> lintInst = ArrayUtils::InstAffectRight;
	    return ret;
	} else if (type && this-> _content-> is<IVoidInfo> ()) {
	    auto ret = this-> clone ();
	    return ret;
	} else if (auto ref = other-> to<IRefInfo> ()) {
	    if (auto arr = ref-> content ()-> to<IArrayInfo> ()) {
		if (!this-> isConst () && arr-> isSame (this)) {
		    auto aux = new (Z0)  IRefInfo (false, this-> clone ());
		    aux-> binopFoo = &ArrayUtils::InstAddr;
		    return aux;
		}
	    }
	} else if (auto en = other-> to<IEnumInfo> ()) {
	    return this-> CompOp (en-> content ());
	} else if (other-> is<INullInfo> ()) {
	    return this-> clone ();	    
	}	    
	return NULL;
    }

    InfoType IArrayInfo::ConstVerif (InfoType other) {
	if (this-> isConst () && !other-> isConst ()) return NULL;
	if (auto ot = other-> to<IArrayInfo> ()) {
	    if (other-> isConst ()) return this;
	    if (!this-> _content-> ConstVerif (ot-> _content)) return NULL;
	    return this;
	} else return NULL;
    }

    Ymir::Tree IArrayInfo::toGeneric () {
	if (!this-> _isStatic) {
	    IInfoType::printConst (false);
	    std::string name = this-> _content-> innerTypeString () + "[]";
	    IInfoType::printConst (true);
	    auto array_type_node = IFinalFrame::getDeclaredType (name.c_str ());
	    if (array_type_node.isNull ()) {
		array_type_node = Ymir::makeStructType (name, 2,
							get_identifier ("len"),
							(new (Z0)  IFixedInfo (true, FixedConst::ULONG))-> toGeneric ().getTree (),
							get_identifier ("ptr"),
							(new (Z0)  IPtrInfo (true, this-> _content-> clone ()))-> toGeneric ().getTree ()
		).getTree ();
		IFinalFrame::declareType (name, array_type_node);
	    }
	    return array_type_node;
	} else {
	    tree begin = build_int_cst_type (integer_type_node, 0);
	    tree len = build_int_cst_type (integer_type_node, this-> _size - 1);
	    auto innerType = this-> _content-> toGeneric ();
	    Ymir::Tree range_type = build_range_type (integer_type_node, fold (begin), fold (len));
	    Ymir::Tree array_type = build_array_type (innerType.getTree (), range_type.getTree ());
	    return array_type;
	}
    }
    
    std::string IArrayInfo::innerTypeString () {
	if (this-> _isStatic) {
	    return Ymir::OutBuffer ("[", this-> _content-> typeString (), " ; ", this-> _size, "]").str ();
	} else {
	    return std::string ("[") + this-> _content-> typeString () + "]";
	}
    }

    std::string IArrayInfo::innerSimpleTypeString () {
	if (this-> _isStatic) {
	    return Ymir::OutBuffer ("A", this-> _size, this-> _content-> simpleTypeString ()).str ();
	} else 
	    return std::string ("A") + this-> _content-> simpleTypeString ();
    }

    InfoType IArrayInfo::getTemplate (ulong i) {
	if (i == 0) return this-> _content;
	if (i == 1 && this-> _isStatic) {
	    return Length ();
	}
	else return NULL;
    }

    bool IArrayInfo::isStatic () {
	return this-> _isStatic;
    }

    void IArrayInfo::isStatic (bool isStatic, ulong size) {
	this-> _isStatic = isStatic;
	this-> _size = size;
    }
    
    const char* IArrayInfo::getId () {
	return IArrayInfo::id ();
    }
    
    namespace ArrayUtils {

	using namespace syntax;
	using namespace Ymir;

	bool isStringType (Tree type) {
	    return type.getTreeCode () == POINTER_TYPE
		&& TYPE_MAIN_VARIANT (TREE_TYPE (type.getTree ())) == char_type_node;
	}
	
	Tree getLen (location_t loc, Expression, Tree tree) {
	    if (tree.getType ().getTreeCode () != RECORD_TYPE) {
		if (isStringType (tree.getType ())) {
		    if (TREE_CONSTANT (tree.getTree ())) {
			auto t = tree.getOperand (0).getOperand (0);
			return build_int_cst_type (long_unsigned_type_node, TREE_STRING_LENGTH (t.getTree ()) - 1);
		    }
		} else {
		    auto range = TYPE_DOMAIN (tree.getType ().getTree ());
		    return convert (long_unsigned_type_node,
				    buildTree (PLUS_EXPR, loc, integer_type_node, 
					       TYPE_MAX_VALUE (range),
					       build_int_cst_type (integer_type_node, 1)
				    ).getTree ()
		    );
		}
	    }	    
	    return getField (loc, tree, "len");		    
	}

	Tree getPtr (location_t loc, Expression, Tree tree) {
	    if (tree.getType ().getTreeCode () != RECORD_TYPE) {
		if (isStringType (tree.getType ())) {
		    return tree;
		} else {
		    return getAddr (tree);
		}
	    } else {
		return getField (loc, tree, "ptr");
	    }
	}	

	Tree buildArray (location_t loc, Tree len, Tree type) {
	    Tree fn = InternalFunction::getMalloc ();
	    auto byteLen = buildTree (
		MULT_EXPR, loc,
		size_type_node,
		fold_convert (size_type_node, len.getTree ()),
		TYPE_SIZE_UNIT (type.getTree ())
	    );
	    
	    tree args [] = {byteLen.getTree ()};	    
	    return build_call_array_loc (loc, build_pointer_type (void_type_node), fn.getTree (), 1, args);	    
	}

	Tree copyArray (location_t loc, Tree dst, Tree src, Tree len, Tree type) {
	    auto byteLen = buildTree (
		MULT_EXPR, loc,
		size_type_node,
		fold_convert (size_type_node, len.getTree ()),
		TYPE_SIZE_UNIT (type.getTree ())
	    );

	    tree argsMemcpy [] = {dst.getTree (), src.getTree (), byteLen.getTree ()};
	    return build_call_array_loc (loc, void_type_node, InternalFunction::getYMemcpy ().getTree (), 3, argsMemcpy);	    

	}
	
	Tree copyArray (location_t loc, Tree dst, Tree src, Tree len, Tree begin, Tree type) {
	    auto byteBegin = buildTree (
		MULT_EXPR, loc,
		size_type_node,
		fold_convert (size_type_node, begin.getTree ()),
		TYPE_SIZE_UNIT (type.getTree ())
	    );

	    auto byteLen = buildTree (
		MULT_EXPR, loc,
		size_type_node,
		fold_convert (size_type_node, len.getTree ()),
		TYPE_SIZE_UNIT (type.getTree ())
	    );

	    dst = buildTree (POINTER_PLUS_EXPR, loc, dst.getType (), dst, byteBegin);
	    
	    tree argsMemcpy [] = {dst.getTree (), src.getTree (), byteLen.getTree ()};
	    return build_call_array_loc (loc, void_type_node, InternalFunction::getYMemcpy ().getTree (), 3, argsMemcpy);	    
	}

	Ymir::Tree buildDupSimple (location_t loc, Tree lexp, Tree rexp, Tree inner) {
	    if (lexp.getTree () == rexp.getTree ()) {
		Ymir::getStackStmtList ().back ().append (
		    Ymir::buildTree (
			MODIFY_EXPR, loc, void_type_node, lexp, rexp
		    )
		);
	    } else {
		auto ptrr = getPtr (loc, NULL, rexp);
		auto lenl = getLen (loc, NULL, lexp);
		auto ptrl = getPtr (loc, NULL, lexp);
		Ymir::TreeStmtList list;
		// tree tmemcopy = builtin_decl_explicit (BUILT_IN_MEMCPY);
		// tree size = fold_build2_loc (loc, MULT_EXPR, size_type_node,
		// 			convert (size_type_node, lenl.getTree ()),
		// 			convert (size_type_node, TYPE_SIZE_UNIT (inner.getTree ()))					
		// );
		
		list.append (copyArray (loc, ptrl, ptrr, lenl, inner));
		//auto result = build_call_expr (tmemcopy, 3, ptrl.getTree (), ptrr.getTree (), size);
		Ymir::getStackStmtList ().back ().append (list.getTree ());
	    }
	    return lexp;
	}
	
	Ymir::Tree buildDup (location_t loc, Tree lexp, Tree rexp, Expression right) {
	    Ymir::TreeStmtList list;
	    ArrayInfo arrayInfo = right-> info-> type-> getIntern ()-> to<IArrayInfo> ();
	    Ymir::Tree inner = arrayInfo-> content ()-> toGeneric ();
	    if (lexp.getType ().getTreeCode () != RECORD_TYPE) return buildDupSimple (loc, lexp, rexp, inner);
	    Ymir::Tree lenl = Ymir::getField (loc, lexp, "len");
	    Ymir::Tree ptrl = Ymir::getField (loc, lexp, "ptr");	
	    Ymir::Tree len, ptrr;
	    
	    if (rexp.getTreeCode () != CALL_EXPR) {
		len = getLen (loc, right, rexp);
		ptrr = getPtr (loc, right, rexp);
	    } else {
		auto aux = makeAuxVar (loc, ISymbol::getLastTmp (), lexp.getType ());
		list.append (buildTree (
		    MODIFY_EXPR, loc, void_type_node, aux, rexp
		));
		    
		len = getField (loc, aux, "len");	    
		ptrr = getField (loc, aux, "ptr");
	    }
		
	    auto allocRet = buildArray (loc, len, inner);
	    list.append (Ymir::buildTree (
		MODIFY_EXPR, loc, void_type_node, lenl.getTree (), len.getTree ()
	    ));
		
	    list.append (Ymir::buildTree (
		MODIFY_EXPR, loc, void_type_node, ptrl.getTree (), allocRet
	    ));
		
	    list.append (copyArray (loc, ptrl, ptrr, len, inner));	    
	    Ymir::getStackStmtList ().back ().append (list.getTree ());
	    return lexp;
	}

	Ymir::Tree InstAffectNull (Word word, InfoType, Expression left, Expression) {
	    auto loc = word.getLocus ();
	    auto ltree = left-> toGeneric ();
	    auto addr = Ymir::getAddr (loc, ltree);
	    tree memsetArgs [] = {addr.getTree (),
				  build_int_cst_type (long_unsigned_type_node, 0),
				  TYPE_SIZE_UNIT (ltree.getType ().getTree ())};

	    Ymir::getStackStmtList ().back ().append (build_call_array_loc (loc, void_type_node, InternalFunction::getYMemset ().getTree (), 3, memsetArgs));
	    return ltree;
	}
	
	Ymir::Tree InstAffect (Word word, InfoType, Expression left, Expression right) {
	    location_t loc = word.getLocus ();
	    auto lexp = left-> toGeneric ();
	    auto rexp = right-> toGeneric ();
	    if (right-> info-> isConst ()) {
		if (auto ref = left-> info-> type-> to <IArrayRefInfo> ()) {
		    if (!ref-> content ()-> isConst ())
			return buildDup (loc, lexp, rexp, right);
		} else if (!left-> info-> isConst ())
		    return buildDup (loc, lexp, rexp, right);
	    }
	    
	    if (lexp.getType () == rexp.getType ()) {		
		return Ymir::buildTree (
		    MODIFY_EXPR, loc, lexp.getType (), lexp, rexp
		);
	    } else if (lexp.getType ().getTreeCode () != RECORD_TYPE) {
		return buildDup (loc, lexp, rexp, right);
	    }
	    
	   
	    Ymir::TreeStmtList list;
	    Ymir::Tree lenl = Ymir::getField (loc, lexp, "len");
	    Ymir::Tree ptrl = Ymir::getField (loc, lexp, "ptr");	
	    
	    auto lenr = getLen (loc, NULL, rexp);
	    auto ptrr = getPtr (loc, NULL, rexp);
	    
	    list.append (Ymir::buildTree (
		MODIFY_EXPR, loc, void_type_node, lenl, lenr
	    ));
	    
	    list.append (Ymir::buildTree (
		MODIFY_EXPR, loc, void_type_node, ptrl, ptrr
	    ));
	    
	    Ymir::getStackStmtList ().back ().append (list.getTree ());		
	    return lexp;			    	    	    
	}	
	
	Ymir::Tree InstAccessInt (Word word, InfoType, Expression left, Expression right) {
	    location_t loc = word.getLocus ();
	    auto lexp = left-> toGeneric ();
	    auto rexp = right-> toGeneric ();

	    ArrayInfo arrayInfo = left-> info-> type-> to<IArrayInfo> ();
	    Ymir::Tree inner = arrayInfo-> content ()-> toGeneric ();
	    if (lexp.getType ().getTreeCode () != RECORD_TYPE) {
		if (isStringType (lexp.getType ())) {
		    return getPointerUnref (loc, lexp, inner, rexp);
		} else 
		    return getArrayRef (loc, lexp, inner, rexp);
	    } else {
		Ymir::Tree ptrl = Ymir::getField (loc, lexp, "ptr");
		return getPointerUnref (loc, ptrl, inner, rexp);
	    }
	}
	
	Tree InstPtr (Word locus, InfoType, Expression expr, Expression) {
	    location_t loc = locus.getLocus ();
	    auto ltree = expr-> toGeneric ();
	    return getPtr (loc, expr, ltree);
	}

	Tree InstLen (Word locus, InfoType, Expression expr, Expression) {
	    location_t loc = locus.getLocus ();
	    auto ltree = expr-> toGeneric ();
	    return getLen (loc, expr, ltree);
	}

	Tree InstAddr (Word locus, InfoType, Expression elem, Expression) {
	    return Ymir::getAddr (locus.getLocus (), elem-> toGeneric ());
	}
	
	Tree InstConcat (Word locus, InfoType retType, Expression left, Expression right) {
	    auto lexp = left-> toGeneric ();
	    auto rexp = right-> toGeneric ();
	    
	    ArrayInfo info = left-> info-> type-> to <IArrayInfo> ();
	    Ymir::Tree inner = info-> content ()-> toGeneric ();
	    location_t loc = locus.getLocus ();
	    Ymir::TreeStmtList list; 

	    auto lenl = getLen (loc, left, lexp);
	    auto lenr = getLen (loc, right, rexp);
	    auto ptrl = getPtr (loc, left, lexp);
	    auto ptrr = getPtr (loc, right, rexp);
	    
	    Ymir::Tree aux = makeAuxVar (loc, ISymbol::getLastTmp (), retType-> toGeneric ());	    

	    auto auxPtr = getField (loc, aux, "ptr");
	    auto auxLen = getField (loc, aux, "len");
	    auto len = buildTree (PLUS_EXPR, loc, lenl.getType (), lenl, lenr);    
	    auto allocRet = buildArray (loc, len, inner);
	    
	    list.append (buildTree (
		MODIFY_EXPR, loc, void_type_node, auxLen, len
	    ));
	    
	    list.append (buildTree (
		MODIFY_EXPR, loc, void_type_node, auxPtr, allocRet
	    ));

	    list.append (copyArray (loc, auxPtr, ptrl, lenl, inner));
	    list.append (copyArray (loc, auxPtr, ptrr, lenr, lenl, inner));	    
	    
	    Ymir::getStackStmtList ().back ().append (list.getTree ());
	    return aux;
	}
	
	Tree InstConcatAff (Word locus, InfoType type, Expression left, Expression right) {
	    location_t loc = locus.getLocus ();
	    auto lexp = left-> toGeneric ();
	    auto aux = InstConcat (locus, type, new (Z0)  ITreeExpression (left-> token, left-> info-> type, lexp), right);
	    Ymir::TreeStmtList list;
	    auto lenl = getLen (loc, left, lexp);
	    auto lenr = getField (loc, aux, "len");
	    auto ptrl = getPtr (loc, left, lexp);
	    auto ptrr = getField (loc, aux, "ptr");

	    list.append (buildTree (
		MODIFY_EXPR, loc, void_type_node, lenl, lenr
	    ));
	    
	    list.append (buildTree (
		MODIFY_EXPR, loc, void_type_node, ptrl, ptrr
	    ));
	    
	    Ymir::getStackStmtList ().back ().append (list.getTree ());
	    
	    return lexp;
	}
	
	Tree InstToArray (Word locus, InfoType, Expression elem, Expression type) {
	    auto loc = locus.getLocus ();
	    auto rexp = elem-> toGeneric ();
	    if (rexp.getType ().getTreeCode () == RECORD_TYPE) {
		return rexp;
	    } else {
		auto toType = type-> info-> type-> toGeneric ();
		if (toType.getTreeCode () == RECORD_TYPE) {
		    Tree auxVar = makeAuxVar (loc, ISymbol::getLastTmp (), toType);
		    auto lenr = getLen (loc, elem, rexp);
		    auto ptrr = getPtr (loc, elem, rexp);
		    auto ptrl = getField (loc, auxVar, "ptr");
		    auto lenl = getField (loc, auxVar, "len");
		    TreeStmtList list;
    
		    list.append (buildTree (
			MODIFY_EXPR, loc, void_type_node, lenl, lenr
		    ));
	    
		    list.append (buildTree (
			MODIFY_EXPR, loc, void_type_node, ptrl, ptrr
		    ));
		    Ymir::getStackStmtList ().back ().append (list.getTree ());
		    return auxVar;
		} else {
		    return rexp;
		}		
	    }
	}

	Tree affectIndex (Word &loc, Tree index, Tree array, Tree var, InfoType arrayType, InfoType varType) {
	    Ymir::Tree elem;
	    if (auto ainfo = arrayType-> to<IArrayInfo> ()) {
		Ymir::Tree inner = (ainfo)-> content ()-> toGeneric ();
		if (array.getType ().getTreeCode () != RECORD_TYPE) {
		    if (isStringType (array.getType ()))
			elem = getPointerUnref (loc.getLocus (), array, char_type_node, index);
		    else
			elem = getArrayRef (loc.getLocus (), array, inner, index);
		} else {
		    auto ptr = Ymir::getField (loc.getLocus (), array, "ptr");
		    elem = getPointerUnref (loc.getLocus (), ptr, inner, index);
		}
	    } else { //StringInfo
		auto inner = (new (Z0) ICharInfo (false))-> toGeneric ();
		if (array.getType ().getTreeCode () != RECORD_TYPE) {
		    if (isStringType (array.getType ()))
			elem = getPointerUnref (loc.getLocus (), array, char_type_node, index);
		    else
			elem = getArrayRef (loc.getLocus (), array, char_type_node, index);
		} else {
		    auto ptr = Ymir::getField (loc.getLocus (), array, "ptr");
		    elem = getPointerUnref (loc.getLocus (), ptr, inner, index);
		}
	    }	    

	    return Ymir::buildTree (
		MODIFY_EXPR, loc.getLocus (),
		void_type_node, var,
		varType-> buildBinaryOp (
		    loc,
		    varType,
		    new (Z0) ITreeExpression (loc, ((ArrayInfo) arrayType)-> content (), elem),
		    new (Z0) ITreeExpression (loc, varType, var)	
		)
	    );
	}
	
	Tree InstApply (Word locus, std::vector <Var> & vars, Block block, Expression iter) {
	    auto loc = locus.getLocus ();
	    auto array = iter-> toGeneric ();
	    auto len = getLen (loc, iter, array);
	    auto intExpr = new (Z0)  IFixed (locus, FixedConst::ULONG);
	    intExpr-> setUValue (0);
	    auto zero = intExpr-> expression ()-> toGeneric ();				    
	    intExpr-> setUValue (1);
	    auto one = intExpr-> expression ()-> toGeneric ();
	    auto var = vars [0]-> toGeneric ();
	    auto it = makeAuxVar (loc, ISymbol::getLastTmp (), long_unsigned_type_node);
	    	    
	    Ymir::TreeStmtList list;
	    
	    list.append (buildTree (MODIFY_EXPR, loc, void_type_node, it, zero));
	    Ymir::Tree bool_expr = buildTree (LT_EXPR, loc, boolean_type_node, it, len);
	    
	    Ymir::Tree test_label = Ymir::makeLabel (loc, "test");
	    Ymir::Tree begin_label = Ymir::makeLabel (loc, "begin");
	    Ymir::Tree end_label = Ymir::makeLabel (loc, "end");

	    Ymir::Tree goto_test = Ymir::buildTree (GOTO_EXPR, iter-> token.getLocus (), void_type_node, test_label);
	    Ymir::Tree goto_end = Ymir::buildTree (GOTO_EXPR, iter-> token.getLocus (), void_type_node, end_label);
	    Ymir::Tree goto_begin = Ymir::buildTree (GOTO_EXPR, iter-> token.getLocus (), void_type_node, begin_label);
	
	    Ymir::Tree test_expr = Ymir::buildTree (COND_EXPR, iter-> token.getLocus (), void_type_node, bool_expr, goto_begin, goto_end);
	    Ymir::Tree begin_label_expr = Ymir::buildTree (LABEL_EXPR, block-> token.getLocus (), void_type_node, begin_label);
	    list.append (goto_test);
	    list.append (begin_label_expr);
	    Ymir::enterBlock ();

	    Ymir::getStackStmtList ().back ().append (affectIndex (locus, it, array, var, iter-> info-> type, vars [0]-> info-> type));
	    auto inner = block-> toGeneric ();
	    Ymir::getStackStmtList ().back ().append (inner);	    
	    Ymir::getStackStmtList ().back ().append (Ymir::buildTree (
	     	MODIFY_EXPR, locus.getLocus (), it.getType (), it,
	     	Ymir::buildTree (
	     	    PLUS_EXPR, locus.getLocus (), it.getType (), it, one
	     	)
	    ));
	    
	    auto begin_part = Ymir::leaveBlock ().bind_expr;
	    list.append (begin_part);	    
	    list.append (goto_test);
	    
	    Ymir::Tree test_label_expr = Ymir::buildTree (LABEL_EXPR, iter-> token.getLocus (), void_type_node, test_label);
	    list.append (test_label_expr);
	    list.append (test_expr);

	    Ymir::Tree end_expr = Ymir::buildTree (LABEL_EXPR, iter-> token.getLocus (), void_type_node, end_label);	
	    list.append (end_expr);

	    return list.getTree ();
	}

	Ymir::Tree InstCall (Word loc, InfoType ret, Expression, Expression paramsExp) {
	    ParamList params = paramsExp-> to <IParamList> ();
	    std::vector <tree> args = params-> toGenericParams (params-> getTreats ());
	    Ymir::TreeStmtList list;
	    auto aux = Ymir::makeAuxVar (loc.getLocus (), ISymbol::getLastTmp (), ret-> toGeneric ());
	    
	    for (auto i : Ymir::r (0, 2)) {
		auto attr = getField (loc.getLocus (), aux, i);
		list.append (buildTree (
		    MODIFY_EXPR, loc.getLocus (), attr.getType (), attr, args [i]
		));
	    }
	    
	    Ymir::getStackStmtList ().back ().append (list.getTree ());
	    return aux;
	}
	
	Ymir::Tree InstInit (Word locus, InfoType type, Expression) {
	    auto loc = locus.getLocus ();
	    auto ltree = Ymir::makeAuxVar (loc, ISymbol::getLastTmp (), type-> toGeneric ());
	    auto addr = Ymir::getAddr (loc, ltree).getTree ();
				  
	    auto size = TYPE_SIZE_UNIT (ltree.getType ().getTree ());
	    tree tmemset = builtin_decl_explicit (BUILT_IN_MEMSET);
	    
	    auto result = build_call_expr (tmemset, 3, addr, integer_zero_node, size);
	    return Ymir::compoundExpr (loc, result, ltree);
	}


    }

}
