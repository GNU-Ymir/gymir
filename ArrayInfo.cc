#include <ymir/semantic/types/_.hh>
#include <ymir/ast/ParamList.hh>
#include <ymir/ast/ConstArray.hh>
#include <ymir/ast/TreeExpression.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/semantic/pack/FinalFrame.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/semantic/utils/ArrayUtils.hh>
#include <ymir/semantic/utils/FixedUtils.hh>
#include <ymir/semantic/utils/TupleUtils.hh>
#include <ymir/semantic/tree/_.hh>
#include <ymir/semantic/pack/InternalFunction.hh>
#include <ymir/semantic/value/FixedValue.hh>
#include <ymir/semantic/value/StringValue.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/semantic/value/ArrayValue.hh>
#include <ymir/semantic/object/AggregateInfo.hh>

namespace semantic {

    using namespace syntax;
    
    IArrayInfo::IArrayInfo (bool isConst, InfoType content) :
	IInfoType (isConst),
	_isStatic (false),
	_size (0),
	_content (content)
    {
    }
    
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
	auto type = right-> info-> type ()-> to<IArrayInfo> ();
	if (type && type-> _content-> isSame (this-> _content)) {
	    if (type-> _isStatic != this-> _isStatic || type-> _size != this-> _size)
		return NULL;
	    
	    auto ret = type-> clone ();
	    ret-> isConst (this-> isConst ());
	    if (type-> ConstVerif (this) == NULL)
		return NULL;
	    
	    ret-> binopFoo = ArrayUtils::InstAffect;
	    return ret;
	} else if (type && type-> _content-> is <IVoidInfo> ()) {
	    if (this-> _isStatic) return NULL;
	    auto ret = this-> clone ();
	    ret-> binopFoo = ArrayUtils::InstAffectNull;
	    return ret;
	} else if (type && this-> _content-> is<IVoidInfo> ()) {
	    if (this-> _isStatic) return NULL;
	    this-> _content = type-> _content-> clone ();
	    auto ret = this-> clone ();
	    ret-> binopFoo = ArrayUtils::InstAffectNull;
	    return ret;
	} else if (right-> info-> type ()-> is<INullInfo> ()) {
	    if (this-> _isStatic) return NULL;
	    auto ret = this-> clone ();
	    ret-> binopFoo = ArrayUtils::InstAffectNull;
	    return ret;
	}
	return NULL;
    }

    InfoType IArrayInfo::AffectRight (Expression left) {
	if (left-> info-> type ()-> is<IUndefInfo> ()) {
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

    InfoType IArrayInfo::UnaryOp (Word token) {
	if (token == Token::AND && this-> isLvalue ()) {
	    auto ret = new (Z0) IPtrInfo (this-> isConst (), this-> clone ());
	    ret-> binopFoo = &FixedUtils::InstAddr;
	    return ret;
	}
	return NULL;
    }
    
    InfoType IArrayInfo::ApplyOp (const std::vector<syntax::Var> & vars) {
	if (vars.size () != 1) return NULL;
	if (!this-> isLvalue () || this-> isConst ()) {
	    vars [0]-> info-> type (this-> _content-> clone ()-> CompOp (vars [0]-> info-> type ()));
	    vars [0]-> info-> type ()-> value () = NULL;
	} else {
	    InfoType ref = new (Z0) IRefInfo (false, this-> _content);
	    auto content = this-> _content-> clone ();
	    content-> value () = NULL;
	    content-> isConst (false);
	    content-> symbol () = this-> symbol ();
	    vars [0]-> info-> type (content-> CompOp (ref));
	}
	
	if (vars [0]-> info-> type () == NULL) return NULL;
	auto ret = this-> clone ();
	ret-> applyFoo = &ArrayUtils::InstApply;
	return ret;
    }

    ApplicationScore IArrayInfo::CallType (Word token, syntax::ParamList params) {
	if (params-> getParams ().size () != 2 && !this-> _isStatic) 
	    return NULL;
	
	auto fst = params-> getParams ()[0];
	auto scd = params-> getParams ()[1];

	auto score = new (Z0) IApplicationScore (token);
	score-> treat.push_back (fst-> info-> type ()-> CompOp (new (Z0) IFixedInfo (true, FixedConst::ULONG)));
	if (!score-> treat.back ()) return NULL;
	
	auto inner = this-> _content-> clone ();
	if (this-> isConst ()) inner-> isConst (true);
	auto info = new (Z0) IPtrInfo (false, inner);
	auto type = scd-> info-> type ()-> CompOp (info);
	if (type) type = type-> ConstVerif (info);
	if (!type) return NULL;
	
	score-> treat.push_back (type);

	auto res = new (Z0) IArrayInfo (type-> isConst (), this-> _content-> clone ());
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
	if (var-> token == "sizeof") return SizeOf ();
	if (var-> token == "typeinfo") return TypeInfo ();
	return NULL;
    }
    
    InfoType IArrayInfo::SizeOf () {
	auto ret = new (Z0)  IFixedInfo (true, FixedConst::UINT);
	ret-> unopFoo = FixedUtils::InstSizeOf;
	return ret;	
    }
    
    InfoType IArrayInfo::Init () {
	auto ret = this-> clone ();
	ret-> unopFoo = ArrayUtils::InstInit;
	return ret;
    }
    
    InfoType IArrayInfo::Ptr () {
	auto inner = this-> _content-> clone ();
	if (this-> isConst ()) inner-> isConst (true);	
	auto ret = new (Z0)  IPtrInfo (this-> isConst (), inner);
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
	auto other = right-> info-> type ();
	if (auto ot = other-> to<IArrayInfo> ()) {
	    if (ot-> _content-> isSame (this-> _content)) {
		auto i = this-> clone ()-> to <IArrayInfo> ();
		i-> isStatic (false, 0);
		i-> isConst (false);
		i-> binopFoo = &ArrayUtils::InstConcat;
		return i;
	    }
	}
	return NULL;
    }

    InfoType IArrayInfo::ConcatAff (syntax::Expression right) {
	if (this-> isConst ()) return NULL;
	if (right-> info-> type ()-> isSame (this)) {
	    auto i = this-> clone ();
	    i-> isConst (false);
	    i-> binopFoo = &ArrayUtils::InstConcatAff;
	    return i;
	}
	return NULL;
    }
    

    InfoType IArrayInfo::Access (syntax::Expression expr, InfoType& treat) {
	treat = expr-> info-> type ()-> CompOp (new (Z0)  IFixedInfo (true, FixedConst::LONG));
	if (treat == NULL) {
	    treat = expr-> info-> type ()-> CompOp (new (Z0)  IFixedInfo (true, FixedConst::ULONG));
	}
	
	if (treat) {
	    auto content = this-> _content-> cloneOnExit ();
	    if (this-> isConst ())
		content-> isConst (true);
	    auto ch = new (Z0)  IArrayRefInfo (this-> isConst () || content-> isConst (), content);
	    ch-> binopFoo = &ArrayUtils::InstAccessInt;
	    if (this-> value ()) 
		ch-> value () = this-> value ()-> AccessOp (expr);	    

	    if (expr-> info-> value () && this-> _isStatic) {
		auto val = expr-> info-> value ()-> to<IFixedValue> ()-> getValue ();
		if (val > (long) this-> _size || val < 0) {
		    Ymir::Error::outOfRange (expr-> token, this-> _size, val);
		}
	    } else {
		if (Table::instance ().hasCurrentContext (Keys::SAFE)) {
		    Ymir::Error::dynamicAccess (expr-> token);
		}
	    }	    
	    
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
	    return ret;
	} else if (type && this-> _content-> is<IVoidInfo> ()) {
	    auto ret = type-> clone ();
	    ret-> isConst (this-> isConst ());
	    ret-> binopFoo = ArrayUtils::InstToArray;
	    return ret;
	} else if (auto ref = other-> to<IRefInfo> ()) {
	    if (auto arr = ref-> content ()-> to<IArrayInfo> ()) {
		if (this-> isLvalue () && arr-> isSame (this)) {
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
	if (auto ot = other-> to<IArrayInfo> ()) {
	    if (other-> isConst ()) return this;
	    if (this-> _content-> ConstVerif (ot-> _content) == NULL) return NULL;
	    else if (this-> _content-> isConst () && !ot-> _content-> isConst ())
		return NULL;
	    else if (this-> isConst () && !ot-> isConst () && !this-> _content-> isConst ())
		return NULL;
	    return this;
	} else return NULL;
    }

    bool IArrayInfo::passingConst (InfoType other) {
	if (IInfoType::passingConst (other)) return true;
	else if (auto type = other-> to <IArrayInfo> ()) {
	    return this-> _content-> passingConst (type-> _content);
	}
	return false;
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

    Ymir::Tree IArrayInfo::genericConstructor () {
	if (this-> value ()) {
	    auto sym = new (Z0) ISymbol (Word::eof (), NULL, this);
	    return this-> value ()-> toYmir (sym)-> toGeneric ();
	}
	
	vec<constructor_elt, va_gc> * elms = NULL;
	auto vtype = this-> toGeneric ();
	if (!this-> _isStatic) {
	    CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (vtype, "len").getTree (), build_int_cst_type (long_unsigned_type_node, 0));
	    CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (vtype, "ptr").getTree (), build_int_cst_type (long_unsigned_type_node, 0));
	} else {
	    for (auto it : Ymir::r (0, this-> _size)) {
		CONSTRUCTOR_APPEND_ELT (elms, size_int (it), this-> _content-> genericConstructor ().getTree ());
	    }
	}
	return build_constructor (vtype.getTree (), elms);
    }

    Ymir::Tree IArrayInfo::genericTypeInfo () {	
	auto inner = this-> _content-> genericTypeInfo ();

	auto type = Table::instance ().getTypeInfoType ()-> TempOp ({});
	auto typeTree = type-> toGeneric ();
	vec <constructor_elt, va_gc> * elms = NULL;
	AggregateInfo array_info_type = NULL;  	
	
	if (this-> _isStatic)
	    array_info_type = Table::instance ().getTypeInfoType (Ymir::Runtime::ARRAY_INFO_STATIC)-> TempOp ({})-> to <IAggregateInfo> ();
	else
	    array_info_type = Table::instance ().getTypeInfoType (Ymir::Runtime::ARRAY_INFO_DYNAMIC)-> TempOp ({})-> to <IAggregateInfo> ();

	auto vtable = array_info_type-> getVtable ();
	CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (typeTree, Keys::VTABLE_FIELD).getTree (), Ymir::getAddr (vtable).getTree ());	   
	CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (typeTree, Ymir::Runtime::VTABLE_FIELD_TYPEINFO).getTree (), build_int_cst_type (long_unsigned_type_node, 0));

	if (this-> _isStatic) {
	    CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (typeTree, Ymir::Runtime::LEN_FIELD_TYPEINFO).getTree (), build_int_cst_type (long_unsigned_type_node, this-> _size));
	    CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (typeTree, Ymir::Runtime::C_O_A_TYPEINFO).getTree (), getAddr (inner).getTree ());	    
	} else {
	    CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (typeTree, Ymir::Runtime::LEN_FIELD_TYPEINFO).getTree (), build_int_cst_type (long_unsigned_type_node, 0));
	    CONSTRUCTOR_APPEND_ELT (elms, Ymir::getFieldDecl (typeTree, Ymir::Runtime::C_O_A_TYPEINFO).getTree (), getAddr (inner).getTree ());
	}

	auto name = Ymir::Runtime::TYPE_INFO_MODULE + "." + this-> simpleTypeString () + Ymir::Runtime::TYPE_INFO_SUFFIX;
	auto glob = Ymir::declareGlobalWeak (name, typeTree, build_constructor (typeTree.getTree (), elms));

	return glob;
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
	    return Ymir::OutBuffer ("A", this-> _size, "A", Mangler::mangle_type (this-> _content, this-> _content-> simpleTypeString ())).str ();
	} else 
	    return std::string ("AA") + Mangler::mangle_type (this-> _content, this-> _content-> simpleTypeString ());
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
    
    bool IArrayInfo::isConst () {
	return IInfoType::isConst ();
    }
    
    void IArrayInfo::isConst (bool isConst) {
	IInfoType::isConst (isConst);
    }

    bool IArrayInfo::needKeepConst () {
	return this-> isConst ();
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
	    Ymir::TreeStmtList list;
	    if (lexp.getTree () == rexp.getTree ()) {
		list.append (
		    Ymir::buildTree (
			MODIFY_EXPR, loc, void_type_node, lexp, rexp
		    )
		);
	    } else {
		auto ptrr = getPtr (loc, NULL, rexp);
		auto lenl = getLen (loc, NULL, lexp);
		auto ptrl = getPtr (loc, NULL, lexp);
		list.append (copyArray (loc, ptrl, ptrr, lenl, inner));
	    }
	    return Ymir::compoundExpr (loc, list.getTree (), lexp);
	}
	
	Ymir::Tree buildDup (location_t loc, Tree lexp, Tree rexp, Expression right) {
	    Ymir::TreeStmtList list;
	    ArrayInfo arrayInfo = right-> info-> type ()-> getIntern ()-> to<IArrayInfo> ();
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
	    return Ymir::compoundExpr (loc, list.getTree (), lexp);
	}

	Ymir::Tree InstAffectNull (Word word, InfoType, Expression left, Expression) {
	    Ymir::TreeStmtList list;
	    auto loc = word.getLocus ();
	    auto ltree = Ymir::getExpr (list, left);
	    
	    auto addr = Ymir::getAddr (loc, ltree);
	    tree memsetArgs [] = {addr.getTree (),
				  build_int_cst_type (long_unsigned_type_node, 0),
				  TYPE_SIZE_UNIT (ltree.getType ().getTree ())};

	    list.append (build_call_array_loc (loc, void_type_node, InternalFunction::getYMemset ().getTree (), 3, memsetArgs));
	    return Ymir::compoundExpr (loc, list.getTree (), ltree);
	}
	
	Ymir::Tree InstAffect (Word word, InfoType, Expression left, Expression right) {
	    location_t loc = word.getLocus ();
	    TreeStmtList list;	    
	    auto lexp = Ymir::getExpr (list, left);
	    auto rexp = Ymir::getExpr (list, right); 
	    
	    if (lexp.getType () == rexp.getType ()) {		
		return Ymir::compoundExpr (loc, list, Ymir::buildTree (
		    MODIFY_EXPR, loc, lexp.getType (), lexp, rexp
		));
	    } else if (lexp.getType ().getTreeCode () != RECORD_TYPE) {
		return Ymir::compoundExpr (loc, list, buildDup (loc, lexp, rexp, right));
	    }

	    if (rexp.getType ().getTreeCode () != RECORD_TYPE) {
		auto lenr = getLen (loc, NULL, rexp);
		auto ptrr = getPtr (loc, NULL, rexp);
		auto ptrl = getField (loc, lexp, "ptr");
		auto lenl = getField (loc, lexp, "len");
		
		list.append (buildTree (
		    MODIFY_EXPR, loc, void_type_node, lenl, lenr
		));
	    
		list.append (buildTree (
		    MODIFY_EXPR, loc, void_type_node, ptrl, ptrr
		));

		return Ymir::compoundExpr (loc, list.getTree (), lexp);
	    } else {
		auto ptrl = Ymir::getAddr (loc, lexp).getTree ();
		auto ptrr = Ymir::getAddr (loc, rexp).getTree ();
		tree tmemcopy = builtin_decl_explicit (BUILT_IN_MEMCPY);
		tree size = TYPE_SIZE_UNIT (lexp.getType ().getTree ());
		auto result = build_call_expr (tmemcopy, 3, ptrl, ptrr, size);
		list.append (result);
		return Ymir::compoundExpr (loc, list, lexp);
	    }
	}	
	
	Ymir::Tree InstAccessInt (Word word, InfoType, Expression left, Expression right) {
	    Ymir::TreeStmtList list;
	    location_t loc = word.getLocus ();
	    auto lexp = Ymir::getExpr (list, left); 
	    auto rexp = Ymir::getExpr (list, right);
	    
	    ArrayInfo arrayInfo = left-> info-> type ()-> to<IArrayInfo> ();
	    Ymir::Tree inner = arrayInfo-> content ()-> toGeneric ();
	    if (lexp.getType ().getTreeCode () != RECORD_TYPE) {
		if (isStringType (lexp.getType ())) {
		    return Ymir::compoundExpr (loc, list.getTree (), getPointerUnref (loc, lexp, inner, rexp));
		} else 
		    return Ymir::compoundExpr (loc, list.getTree (), getArrayRef (loc, lexp, inner, rexp));
	    } else {
		Ymir::Tree ptrl = Ymir::getField (loc, lexp, "ptr");
		return Ymir::compoundExpr (loc, list.getTree (), getPointerUnref (loc, ptrl, inner, rexp));
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
	    Ymir::TreeStmtList list; 
	    auto lexp = Ymir::getExpr (list, left);
	    auto rexp = Ymir::getExpr (list, right);
	    
	    ArrayInfo info = left-> info-> type ()-> to <IArrayInfo> ();
	    Ymir::Tree inner = info-> content ()-> toGeneric ();
	    location_t loc = locus.getLocus ();


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
	    
	    return Ymir::compoundExpr (loc, list.getTree (), aux);
	}
	
	Tree InstConcatAff (Word locus, InfoType type, Expression left, Expression right) {
	    location_t loc = locus.getLocus ();
	    Ymir::TreeStmtList list;
	    auto lexp = Ymir::getExpr (list, left);
	    
	    auto aux = InstConcat (locus, type, new (Z0)  ITreeExpression (left-> token, left-> info-> type (), lexp), right);
	    
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
	    	    
	    return Ymir::compoundExpr (loc, list.getTree (), lexp);
	}
	
	Tree InstToArray (Word locus, InfoType, Expression elem, Expression type) {
	    auto loc = locus.getLocus ();
	    TreeStmtList list;
	    auto rexp = Ymir::getExpr (list, elem);
	    auto toType = type-> info-> type ()-> toGeneric ();

	    if (toType != rexp.getType () && toType.getTreeCode () == RECORD_TYPE) {		
		Tree auxVar = makeAuxVar (loc, ISymbol::getLastTmp (), toType);
	       		
		auto lenr = getLen (loc, elem, rexp);
		auto ptrr = getPtr (loc, elem, rexp);
		auto ptrl = getField (loc, auxVar, "ptr");
		auto lenl = getField (loc, auxVar, "len");
		
		list.append (buildTree (
		    MODIFY_EXPR, loc, void_type_node, lenl, lenr
		));
		
		list.append (buildTree (
		    MODIFY_EXPR, loc, void_type_node, ptrl, ptrr
		));
		
		return Ymir::compoundExpr (loc, list.getTree (), auxVar);
	    } else {
		return Ymir::compoundExpr (loc, list, rexp);
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
		    new (Z0) ITreeExpression (loc, arrayType-> to <IArrayInfo> ()-> content (), elem),
		    new (Z0) ITreeExpression (loc, varType, var)	
		)
	    );
	}
	
	Tree InstApply (Word locus, InfoType, std::vector <Var> & vars, Block block, Expression iter) {
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
	    	    
	    Ymir::TreeStmtList list, begin_part;
	    
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

	    begin_part.append (affectIndex (locus, it, array, var, iter-> info-> type (), vars [0]-> info-> type ()));

	    auto inner = block-> toGeneric ();
	    begin_part.append (inner);	    
	    begin_part.append (Ymir::buildTree (
	     	MODIFY_EXPR, locus.getLocus (), it.getType (), it,
	     	Ymir::buildTree (
	     	    PLUS_EXPR, locus.getLocus (), it.getType (), it, one
	     	)
	    ));
	    
	    list.append (begin_part.getTree ());	    
	    list.append (goto_test);
	    
	    Ymir::Tree test_label_expr = Ymir::buildTree (LABEL_EXPR, iter-> token.getLocus (), void_type_node, test_label);
	    list.append (test_label_expr);
	    list.append (test_expr);

	    Ymir::Tree end_expr = Ymir::buildTree (LABEL_EXPR, iter-> token.getLocus (), void_type_node, end_label);	
	    list.append (end_expr);
	    return list.getTree ();
	}

	Ymir::Tree InstCall (Word loc, InfoType ret, Expression, Expression paramsExp, ApplicationScore) {
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
	    
	    return Ymir::compoundExpr (loc.getLocus (), list.getTree (), aux);
	}
	
	Ymir::Tree InstInit (Word, InfoType type, Expression) {
	    return type-> genericConstructor ();
	}

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

	Ymir::Tree InstTypeInfo (Word, InfoType type, Expression elem) {
	    type-> unopFoo = getAndRemoveBack (type-> nextUnop);	    
	    auto arrayInfo = elem-> info-> type ()-> to <IArrayInfo> ();	   
	    return arrayInfo-> genericTypeInfo ();
	}

	Ymir::Tree InstEquals (Word locus, InfoType, Expression left, Expression right) {
	    Ymir::TreeStmtList list;
	    auto ltree = left-> toGeneric ();
	    auto rtree = right-> toGeneric ();
	    auto ret = Ymir::callLib (locus.getLocus (), Ymir::Runtime::MEM_EQ_ARRAY, integer_type_node, {ltree, rtree});
	    tree_code code = OperatorUtils::toGeneric (locus);
	    return Ymir::buildTree (
		code, locus.getLocus (), boolean_type_node, integer_zero_node, ret
	    );
	}
	
    }

    IArrayValue::IArrayValue () {}
    
    void IArrayValue::addValue (Value val) {
	this-> _values.push_back (val);
    }

    Value IArrayValue::BinaryOp (Word op, Value other) {
	if (other == NULL) return NULL;
	if (op == Token::TILDE) {
	    if (auto ot = other-> to <IArrayValue> ()) {
		auto ret = new (Z0) IArrayValue ();
		for (auto & it : this-> _values) ret-> addValue (it);
		for (auto & it : ot-> _values) ret-> addValue (it);
		return ret;
	    }	    
	}
	return NULL;
    }

    Value IArrayValue::AccessOp (syntax::Expression expr) {
	if (expr-> info-> value () == NULL) return NULL;
	else if (auto fix = expr-> info-> value ()-> to <IFixedValue> ()) {
	    auto index = fix-> getUValue ();
	    if (index >= this-> _values.size ()) {
		Ymir::Error::overflowArray (expr-> token, index, this-> _values.size ());
		return NULL;
	    } else {
		return this-> _values [index]-> clone ();
	    }
	}
	return NULL;
    }

    const char* IArrayValue::getId () {
	return IArrayValue::id ();
    }

    Value IArrayValue::getLen () {
	return NULL;
    }

    std::string IArrayValue::toString () {
	Ymir::OutBuffer buf ("[");
	for (auto it : Ymir::r (0, this-> _values.size ())) {
	    buf.write (this-> _values [it]-> toString ());
	    if (it < (int) this-> _values.size () - 1)
		buf.write (", ");
	}
	return buf.str ();
    }

    syntax::Expression IArrayValue::toYmir (Symbol sym) {
	vec <constructor_elt, va_gc> * elms = NULL;
	int i = 0;
	auto vtype = sym-> type ()-> toGeneric ();
	auto arrayInner = sym-> type ()-> to <IArrayInfo> ()-> content ();
	for (auto it : this-> _values) {
	    auto sym_ = new (Z0) ISymbol (sym-> sym, NULL, arrayInner);
	    CONSTRUCTOR_APPEND_ELT (elms, size_int (i), it-> toYmir (sym_)-> toGeneric ().getTree ());
	    i ++;
	}
	return new (Z0) ITreeExpression (sym-> sym, sym-> type (), build_constructor (vtype.getTree (), elms));
    }

    bool IArrayValue::equals (Value val) {
	if (auto ot = val-> to <IArrayValue> ()) {
	    if (this-> _values.size () != ot-> _values.size ()) return false;
	    for (auto it : Ymir::r (0, this-> _values.size ())) {
		if (!this-> _values [it]-> equals (ot-> _values [it]))
		    return false;
	    }
	    return true;
	} else return false;
    }

    Value IArrayValue::clone () {
	auto other = new (Z0) IArrayValue ();
	for (auto & it : this-> _values) {
	    other-> addValue (it-> clone ());
	}
	return other;
    }
    
}
