#include <ymir/semantic/types/_.hh>
#include <ymir/semantic/utils/StringUtils.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/ast/TreeExpression.hh>
#include <ymir/ast/ParamList.hh>
#include <ymir/errors/Error.hh>
#include <ymir/semantic/pack/FinalFrame.hh>
#include <ymir/semantic/pack/InternalFunction.hh>

namespace semantic {

    bool IStringInfo::__initStringTypeNode__ = false;
    
    IStringInfo::IStringInfo (bool isConst) :
	IInfoType (isConst)
    {}

    bool IStringInfo::isSame (InfoType other) {
	return other-> is<IStringInfo> ();
    }

    std::string IStringInfo::innerTypeString () {
	return "string";
    }

    std::string IStringInfo::simpleTypeString () {
	return "s";
    }

    InfoType IStringInfo::BinaryOp (Word op, syntax::Expression right) {
	if (op == Token::EQUAL) return Affect (right);
	else if (op == Token::TILDE) return Concat (right);
	else if (op == Token::TILDE_EQUAL) return ConcatAff (right);
	else return NULL;
    }
    
    InfoType IStringInfo::BinaryOpRight (Word op, syntax::Expression left) {
	if (op == Token::EQUAL) return AffectRight (left);
	return NULL;
    }

    InfoType IStringInfo::AccessOp (Word, syntax::ParamList left, std::vector <InfoType> & treats) {
	if (left-> getParams ().size () == 1) {
	    return Access (left-> getParams () [0], treats [0]);
	}
	return NULL;
    }
    
    InfoType IStringInfo::Access (syntax::Expression expr, InfoType& treat) {
	treat = expr-> info-> type-> CompOp (new (GC) IFixedInfo (true, FixedConst::LONG));
	if (treat == NULL) {
	    treat = expr-> info-> type-> CompOp (new (GC) IFixedInfo (true, FixedConst::ULONG));
	}
	
	if (treat) {	    
	    auto ch = new ICharInfo (this-> isConst ());
	    ch-> binopFoo = &StringUtils::InstAccessInt;
	    return ch;
	}
	return NULL;
    }
        
    InfoType IStringInfo::clone () {
	return new IStringInfo (this-> isConst ());
    }

    InfoType IStringInfo::DotOp (syntax::Var var) {
	if (var-> hasTemplate ()) return NULL;
	if (var-> token == "ptr") return Ptr ();
	if (var-> token == "len") return Length ();
	return NULL;
    }

    InfoType IStringInfo::CompOp (InfoType other) {
	if (this-> isSame (other)) {
	    auto ret = this-> clone ();
	    ret-> binopFoo = &StringUtils::InstToString;
	    return ret;
	} else if (auto ref = other-> to<IRefInfo> ()) {
	    if (!this-> isConst () && this-> isSame (ref-> content ())) {
		auto ret = new (GC) IRefInfo (false, this-> clone ());
		ret-> binopFoo = &StringUtils::InstAddr;
		return ret;
	    }
	}
	return NULL;
    }
    
    InfoType IStringInfo::Ptr () {	
	auto ret = new IPtrInfo (this-> isConst (), new ICharInfo (this-> isConst ()));
	ret-> unopFoo = &StringUtils::InstPtr;
	return ret;
    }

    InfoType IStringInfo::Length () {
	auto ret = new IFixedInfo (true, FixedConst::ULONG);
	return ret;
    }
    
    InfoType IStringInfo::AffectRight (syntax::Expression left) {
	if (left-> info-> type-> is<IUndefInfo> ()) {
	    auto i = this-> clone ();
	    i-> isConst (false);
	    i-> binopFoo = StringUtils::InstAff;
	    return i;
	}
	return NULL;
    }

    InfoType IStringInfo::Affect (syntax::Expression right) {
	if (right-> info-> type-> is <IStringInfo> ()) {
	    auto i = this-> clone ();
	    i-> binopFoo = &StringUtils::InstAff;
	    return i;
	}
	return NULL;
    }

    InfoType IStringInfo::Concat (syntax::Expression right) {
	if (right-> info-> type-> is <IStringInfo> ()) {
	    auto i = this-> clone ();
	    i-> isConst (false);
	    i-> binopFoo = &StringUtils::InstConcat;
	    return i;
	}
	return NULL;
    }

    InfoType IStringInfo::ConcatAff (syntax::Expression right) {
	if (this-> isConst ()) return NULL;
	if (right-> info-> type-> is <IStringInfo> ()) {
	    auto i = this-> clone ();
	    i-> isConst (false);
	    i-> binopFoo = &StringUtils::InstConcatAff;
	    return i;
	}
	return NULL;
    }
    
    const char* IStringInfo::getId () {
	return IStringInfo::id ();
    }

    InfoType IStringInfo::ConstVerif (InfoType other) {
	if (this-> isConst () && !other-> isConst ()) return NULL;
	else if (!this-> isConst () && other-> isConst ()) {
	    this-> isConst (true);
	}
	return this;	
    }

    Ymir::Tree IStringInfo::toGeneric () {
	return toGenericStatic ();
    }
    
    Ymir::Tree IStringInfo::toGenericStatic () {
	auto string_type_node = IFinalFrame::getDeclaredType ("string");
	if (string_type_node.isNull ()) {
	    string_type_node = Ymir::makeStructType ("string", 2,
						     get_identifier ("len"),
						     (new IFixedInfo (true, FixedConst::ULONG))-> toGeneric ().getTree (),
						     get_identifier ("ptr"),
						     (new IPtrInfo (true, new ICharInfo (true)))-> toGeneric ().getTree ()
	    ).getTree ();
	    IFinalFrame::declareType ("string", string_type_node);	    
	}
	return string_type_node;
    }

    namespace StringUtils {
	using namespace syntax;
	using namespace Ymir;


	Tree getLen (location_t loc, Expression expr, Tree tree) {
	    if (auto cst = expr-> to <IString> ()) {
		auto intExpr = new (GC) IFixed (cst-> token, FixedConst::ULONG);
		intExpr-> setUValue (cst-> getStr ().length ());
		auto lenExpr = (Fixed) intExpr-> expression ();			
		return lenExpr-> toGeneric ();
	    } else {
		return getField (loc, tree, "len");
	    }
	}

	Tree getPtr (location_t loc, Expression, Tree tree) {
	    if (tree.getType ().getTreeCode () != RECORD_TYPE) {
		return tree;
	    } else {
		return getField (loc, tree, "ptr");
	    }
	}
	
	Tree buildString (location_t loc, Tree len) {
	    Tree fn = InternalFunction::getMalloc ();
	    auto byteLen = buildTree (
		MULT_EXPR, loc,
		size_type_node,
		fold_convert (size_type_node, len.getTree ()),
		TYPE_SIZE_UNIT (char_type_node)
	    );
	    
	    tree args [] = {byteLen.getTree ()};	    
	    return build_call_array_loc (loc, build_pointer_type (void_type_node), fn.getTree (), 1, args);	    
	}

	Tree copyString (location_t loc, Tree dst, Tree src, Tree len) {
	    auto byteLen = buildTree (
		MULT_EXPR, loc,
		size_type_node,
		fold_convert (size_type_node, len.getTree ()),
		TYPE_SIZE_UNIT (char_type_node)
	    );
	    
	    tree argsMemcpy [] = {dst.getTree (), src.getTree (), byteLen.getTree ()};
	    return build_call_array_loc (loc, void_type_node, InternalFunction::getYMemcpy ().getTree (), 3, argsMemcpy);	    
	}
	
	Tree buildDup (location_t loc, Tree lexp, Tree rexp, Expression cst) {
	    TreeStmtList list;	    
	    Tree lenl = getField (loc, lexp, "len");	    
	    Tree ptrl = getField (loc, lexp, "ptr");
	    Tree len, ptrr;

	    if (rexp.getTreeCode () != CALL_EXPR) {
		len = getLen (loc, cst, rexp);
		ptrr = getPtr (loc, cst, rexp);		
	    } else {
		auto aux = makeAuxVar (loc, ISymbol::getLastTmp (), lexp.getType ());
		list.append (buildTree (
		    MODIFY_EXPR, loc, void_type_node, aux, rexp
		));
		
		len = getField (loc, aux, "len");	    
		ptrr = getField (loc, aux, "ptr");
	    }

	    auto allocRet = buildString (loc, len);
	    list.append (buildTree (
		MODIFY_EXPR, loc, void_type_node, lenl.getTree (), len.getTree ()
	    ));
		
	    list.append (buildTree (
		MODIFY_EXPR, loc, void_type_node, ptrl.getTree (), allocRet
	    ));
		
	    list.append (copyString (loc, ptrl, ptrr, len));
	    
	    getStackStmtList ().back ().append (list.getTree ());
	    return lexp;	    
	}
	
	Tree InstAff (Word word, InfoType, Expression left, Expression right) {
	    location_t loc = word.getLocus ();	   
	    auto lexp = left-> toGeneric ();
	    auto rexp = right-> toGeneric ();	    
	    if (right-> info-> isConst ()) {
		if (!left-> info-> isConst ())
		    return buildDup (loc, lexp, rexp, right);
	    }

	    if (rexp.getTreeCode () != STRING_CST) {
		getStackStmtList ().back ().append (buildTree (
		    MODIFY_EXPR, loc, void_type_node, lexp, rexp
		));
		return lexp;
	    }

	    TreeStmtList list;
	    Tree lenl = getField (loc, lexp, "len");
	    Tree ptrl = getField (loc, lexp, "ptr");	
	    
	    auto lenr = getLen (loc, right, rexp);
	    auto ptrr = getPtr (loc, right, rexp);	    
	    
	    list.append (buildTree (
		MODIFY_EXPR, loc, void_type_node, lenl, lenr)
	    );
	    
	    list.append (buildTree (
		MODIFY_EXPR, loc, void_type_node, ptrl, ptrr)
	    );
	    
	    getStackStmtList ().back ().append (list.getTree ());
		
	    return lexp;			    
	}
	
	Tree InstPtr (Word locus, InfoType, Expression expr) {
	    location_t loc = locus.getLocus ();
	    if (expr-> is<IString> ()) {
		return expr-> toGeneric ();
	    } else {
		auto lexp = expr-> toGeneric ();
		return getField (loc, lexp, "ptr");
	    }
	}

	Tree InstToString (Word locus, InfoType, Expression elem, Expression type) {
	    auto rexp = elem-> toGeneric ();
	    if (rexp.getTreeCode () == CALL_EXPR)
		return rexp;
	    
	    if (elem-> info-> isConst ()) {
		location_t loc = locus.getLocus ();
		Tree auxVar = makeAuxVar (loc, ISymbol::getLastTmp (), type-> info-> type-> toGeneric ());
		if (!type-> info-> isConst ()) {
		    return buildDup (loc, auxVar, rexp, elem);
		} else {
		    TreeStmtList list;
		    Tree lenr = getLen (loc, elem, rexp);
		    Tree ptrr = getPtr (loc, elem, rexp);

		    Tree aux = makeAuxVar (loc, ISymbol::getLastTmp (), IStringInfo::toGenericStatic ());

		    Tree lenl = getField (loc, aux, "len");
		    Tree ptrl = getField (loc, aux, "ptr");
		    list.append (buildTree (
			MODIFY_EXPR, loc, void_type_node, lenl, lenr)
		    );
		    
		    list.append (buildTree (
			MODIFY_EXPR, loc, void_type_node, ptrl, ptrr)
		    );

		    getStackStmtList ().back ().append (list.getTree ());
		    return aux;
		}
	    } else {
		return elem-> toGeneric ();
	    }
	}
	
	Tree InstConcat (Word locus, InfoType, Expression left, Expression right) {
	    location_t loc = locus.getLocus ();
	    Ymir::TreeStmtList list; 
	    auto lexp = left-> toGeneric ();
	    auto rexp = right-> toGeneric ();

	    auto lenl = getLen (loc, left, lexp);
	    auto lenr = getLen (loc, right, rexp);
	    auto ptrl = getPtr (loc, left, lexp);
	    auto ptrr = getPtr (loc, right, rexp);
	    
	    Ymir::Tree aux = makeAuxVar (loc, ISymbol::getLastTmp (), IStringInfo::toGenericStatic ());	    

	    auto auxPtr = getField (loc, aux, "ptr");
	    auto auxLen = getField (loc, aux, "len");
	    auto len = buildTree (PLUS_EXPR, loc, lenl.getType (), lenl, lenr);    
	    auto allocRet = buildString (loc, len);
	    
	    list.append (buildTree (
		MODIFY_EXPR, loc, void_type_node, auxLen, len
	    ));
	    
	    list.append (buildTree (
		MODIFY_EXPR, loc, void_type_node, auxPtr, allocRet
	    ));

	    list.append (copyString (loc, auxPtr, ptrl, lenl));
	    list.append (copyString (loc, build2 (POINTER_PLUS_EXPR, auxPtr.getType ().getTree (),
				auxPtr.getTree (), convert_to_ptrofftype (lenl.getTree ())),
			ptrr, lenr)
	    );
	    
	    Ymir::getStackStmtList ().back ().append (list.getTree ());
	    return aux;
	}

	Tree InstConcatAff (Word locus, InfoType type, Expression left, Expression right) {
	    location_t loc = locus.getLocus ();
	    auto lexp = left-> toGeneric ();
	    auto aux = InstConcat (locus, type, new (GC) ITreeExpression (left-> token, left-> info-> type, lexp), right);
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

	Ymir::Tree InstAccessInt (Word word, InfoType, Expression left, Expression right) {
	    location_t loc = word.getLocus ();
	    auto lexp = left-> toGeneric ();
	    auto rexp = right-> toGeneric ();
	    if (left-> is<IString> ()) {
		return getPointerUnref (loc, lexp, char_type_node, rexp);
	    } else {
		Ymir::Tree ptrl = Ymir::getField (loc, lexp, "ptr");
		return getPointerUnref (loc, ptrl, char_type_node, rexp);
	    }
	}


	Ymir::Tree InstAddr (Word locus, InfoType, Expression elem, Expression) {
	    return Ymir::getAddr (locus.getLocus (), elem-> toGeneric ());
	}	

    }

    
}
