#include <ymir/syntax/Keys.hh>
#include <ymir/ast/_.hh>
#include <ymir/semantic/tree/Tree.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/semantic/types/_.hh>
#include <ymir/errors/Error.hh>
#include <ymir/semantic/pack/FinalFrame.hh>
#include <ymir/semantic/pack/InternalFunction.hh>
#include "print-tree.h"

namespace syntax {
    using namespace semantic;
    
    Ymir::Tree IInstruction::toGeneric () {
	Ymir::Error::assert ((std::string ("TODO generic") + this-> getId ()).c_str  ());
	return NULL;
    }
    
    Ymir::Tree IBlock::toGenericNoEntry () {
	Ymir::TreeStmtList list;
	for (auto it : this-> insts) {
	    list.append (it-> toGeneric ());
	}
	return list.getTree ();
    }
    
    Ymir::Tree IBlock::toGeneric () {
	Ymir::enterBlock ();
	for (auto it : this-> insts) {	    
	    Ymir::getStackStmtList ().back ().append (it-> toGeneric ());    
	}
	
	auto ret = Ymir::leaveBlock ();
	return ret.bind_expr;
    }

    Ymir::Tree IVarDecl::toGeneric () {
	Ymir::TreeStmtList list;
	for (int i = 0 ; i < (int) this-> decls.size () ; i++) {
	    auto var = this-> decls [i];
	    auto aff = this-> insts [i];
	    auto type_tree = var-> info-> type-> toGeneric ();
	    Ymir::Tree decl = build_decl (
		var-> token.getLocus (),
		VAR_DECL,
		get_identifier (var-> token.getStr ().c_str ()),
		type_tree.getTree ()
	    );
	    
	    DECL_CONTEXT (decl.getTree ()) = IFinalFrame::currentFrame ().getTree ();	    
	    var-> info-> treeDecl (decl);
	    Ymir::getStackVarDeclChain ().back ().append (decl);
	    list.append (buildTree (DECL_EXPR, var-> token.getLocus (), void_type_node, decl));
	    
	    if (aff != NULL) {
		list.append (aff-> toGeneric ());
	    }

	}
	return list.getTree ();
    }

    Ymir::Tree IFixed::toGeneric () {
	switch (this-> type) {
	case FixedConst::BYTE : return build_int_cst_type (signed_char_type_node, this-> value);
	case FixedConst::SHORT : return build_int_cst_type (short_integer_type_node, this-> value);
	case FixedConst::INT : return build_int_cst_type (integer_type_node, this-> value);
	case FixedConst::LONG : return build_int_cst_type (long_integer_type_node, this-> value);		

	case FixedConst::UBYTE : return build_int_cst_type (unsigned_char_type_node, this-> uvalue);
	case FixedConst::USHORT : return build_int_cst_type (short_unsigned_type_node, this-> uvalue);		
	case FixedConst::UINT : return build_int_cst_type (unsigned_type_node, this-> uvalue);
	case FixedConst::ULONG : return build_int_cst_type (long_unsigned_type_node, this-> uvalue);
	}
	
	return Ymir::Tree ();
    }

    Ymir::Tree IChar::toGeneric () {
	return build_int_cst_type (unsigned_char_type_node, this-> code);
    }
    
    Ymir::Tree IBool::toGeneric () {
	return build_int_cst_type (unsigned_char_type_node, this-> token == Keys::TRUE_);
    }

    Ymir::Tree IFloat::toGeneric () {
	REAL_VALUE_TYPE real_value;
	Ymir::Tree type = this-> info-> type-> toGeneric ();
	real_from_string3 (&real_value, this-> totale.c_str (), TYPE_MODE (type.getTree ()));
	return build_real (type.getTree (), real_value);
    }
    
    Ymir::Tree IVar::toGeneric () {
	return this-> info-> treeDecl ();
    }

    Ymir::Tree IBinary::toGeneric () {
	return this-> info-> type-> buildBinaryOp (
	    this-> token,
	    this-> info-> type,
	    this-> left,
	    this-> right
	);
    }

    Ymir::Tree ICast::toGeneric () {
	if (this-> info-> type-> unopFoo) {
	    return this-> info-> type-> buildUnaryOp (
		this-> token,
		this-> info-> type,
		this-> expr
	    );
	} else {
	    return this-> info-> type-> buildBinaryOp (
		this-> token,
		this-> info-> type,
		this-> expr,
		new (GC) ITreeExpression (this-> token, this-> info-> type, Ymir::Tree ())
	    );
	}
    }
    
    Ymir::Tree IUnary::toGeneric () {
	if (this-> info-> type-> unopFoo) {
	    return this-> info-> type-> buildUnaryOp (
		this-> token,
		this-> info-> type,
		this-> elem
	    );
	} else {
	    return this-> info-> type-> buildBinaryOp (
		this-> token,
		this-> info-> type,
		this-> elem,
		new (GC) ITreeExpression (this-> token, this-> info-> type, Ymir::Tree ())
	    );
	}
    }
    
    Ymir::Tree IAccess::toGeneric () {
	if (this-> info-> type-> binopFoo) {
	    return this-> info-> type-> buildBinaryOp (
		this-> token,
		this-> info-> type,
		this-> left,
		this-> params-> getParams () [0]
	    );
	} else {
	    return this-> info-> type-> buildMultOp (
		this-> token,
		this-> info-> type,
		this-> left,
		this-> params
	    );
	}
    }
    
    std::vector <tree> IParamList::toGenericParams (std::vector <semantic::InfoType> treat) {
	std::vector <tree> params (this-> params.size ());
	for (uint i = 0 ; i < this-> params.size () ; i++) {
	    Ymir::Tree elist;
	    if (treat [i] && treat [i]-> binopFoo) {		
		elist = treat [i]-> buildCastOp (
		    this-> params [i]-> token,
		    treat [i],
		    this-> params [i],
		    new (GC) ITreeExpression (this-> params [i]-> token, treat [i], Ymir::Tree ())
		);
	    } else {
		elist = this-> params [i]-> toGeneric ();
	    }
	    params [i] = elist.getTree ();
	}
	return params;
    }
    
    Ymir::Tree IPar::toGeneric () {
	std::vector <tree> args = this-> params-> toGenericParams (this-> _score-> treat);
	// if (this-> _score-> left) {	    
	// }

	Ymir::Tree fn = this-> _score-> proto-> toGeneric ();
	return build_call_array_loc (this-> token.getLocus (),
				     this-> _score-> ret-> toGeneric ().getTree (),
				     fn.getTree (),
				     args.size (),
				     args.data ()
	);
    }
    
    Ymir::Tree IString::toGeneric () {
	return build_string_literal (this-> content.length () + 1, this-> content.c_str ());       
    }

    Ymir::Tree IConstArray::toGeneric () {
	ArrayInfo info = this-> info-> type-> to<IArrayInfo> ();
	Ymir::Tree innerType = info-> content ()-> toGeneric ();
	auto intExpr = new IFixed (this-> token, FixedConst::ULONG);
	intExpr-> setUValue (this-> params.size () - 1);
	auto lenExpr = intExpr-> expression ();			
	auto len = lenExpr-> toGeneric ();
	intExpr-> setUValue (0);
	auto begin = intExpr-> toGeneric ();
	
	Ymir::TreeStmtList list;

	Ymir::Tree range_type = build_range_type (integer_type_node, fold (begin.getTree ()), fold (len.getTree ()));
	Ymir::Tree array_type = build_array_type (innerType.getTree (), range_type.getTree ());
	
	Ymir::Tree aux = Ymir::makeAuxVar (this-> token.getLocus (),
					   ISymbol::getLastTmp (),
					   array_type
	);
	
	for (uint i = 0 ; i < this-> params.size () ; i++) {
	    Ymir::Tree ref = Ymir::getArrayRef (this-> token.getLocus (), aux, innerType, i);
	    auto left = new ITreeExpression (this-> token, info-> content (), ref);
	    list.append (Ymir::buildTree (MODIFY_EXPR, this-> token.getLocus (),
					  void_type_node,
					  ref, 
					  this-> casters [i]-> buildCastOp (this-> token, this-> casters [i], this-> params [i], left)
	    ));
	}

	Ymir::getStackStmtList ().back ().append (list.getTree ());
	return aux;
    }

    Ymir::Tree IDot::toGeneric () {
	if (this-> right-> is<IVar> ()) {
	    return this-> info-> type-> buildUnaryOp (
		this-> token,
		this-> info-> type,
		this-> left
	    );
	} else {
	    return this-> info-> type-> buildBinaryOp (
		this-> token,
		this-> info-> type,
		this-> left,
		this-> right
	    );
	}
    }    

    Ymir::Tree ITreeExpression::toGeneric () {
	return this-> _content;
    }

    Ymir::Tree INull::toGeneric () {
	return build_int_cst_type (long_unsigned_type_node, 0);
    }

    Ymir::Tree IIf::toGeneric () {
	if (this-> test != NULL) {
	    Ymir::Tree bool_expr = this-> test-> toGeneric ();
	    Ymir::Tree thenLabel = Ymir::makeLabel (this-> token.getLocus (), "then");
	    Ymir::Tree endLabel  = Ymir::makeLabel (this-> token.getLocus (), "end_if");
	    Ymir::Tree goto_then = Ymir::buildTree (GOTO_EXPR, this-> test-> token.getLocus (), void_type_node, thenLabel);
	    Ymir::Tree goto_end = Ymir::buildTree (GOTO_EXPR, this-> test-> token.getLocus (), void_type_node, endLabel);
	    Ymir::Tree goto_else, elseLabel;
	    
	    if (this-> else_) {
		elseLabel = Ymir::makeLabel (this-> token.getLocus (), "else");
		goto_else = Ymir::buildTree (GOTO_EXPR, this-> test-> token.getLocus (), void_type_node, elseLabel);
	    } else {
		goto_else = goto_end;
	    }
	    
	    Ymir::TreeStmtList list;
	    Ymir::Tree cond_expr = Ymir::buildTree (COND_EXPR, this-> test-> token.getLocus (), void_type_node, bool_expr, goto_then, goto_else);
	    list.append (cond_expr);
	    
	    Ymir::Tree then_label_expr = Ymir::buildTree (LABEL_EXPR, this-> block-> token.getLocus (), void_type_node, thenLabel);

	    list.append (then_label_expr);
	    Ymir::Tree then_part = this-> block-> toGeneric ();
	    list.append (then_part);
	    list.append (goto_end);
	    
	    if (this-> else_) {		
		Ymir::Tree else_label_expr = Ymir::buildTree (LABEL_EXPR, this-> else_-> token.getLocus (), void_type_node, elseLabel);
		list.append (else_label_expr);
		Ymir::Tree else_part = this-> else_-> toGeneric ();
		list.append (else_part);
		list.append (goto_end);
	    }

	    Ymir::Tree endif_label_expr = Ymir::buildTree (LABEL_EXPR, this-> token.getLocus (), void_type_node, endLabel);
	    list.append (endif_label_expr);

	    return  list.getTree ();
	} else {
	    return this-> block-> toGeneric ();
	}
    }

    Ymir::Tree IWhile::toGeneric () {
	Ymir::TreeStmtList list;
	Ymir::Tree bool_expr = this-> test-> toGeneric ();
	Ymir::Tree test_label = Ymir::makeLabel (this-> token.getLocus (), "test");
	Ymir::Tree begin_label = Ymir::makeLabel (this-> token.getLocus (), "begin");
	Ymir::Tree end_label = Ymir::makeLabel (this-> token.getLocus (), "end");

	Ymir::Tree goto_test = Ymir::buildTree (GOTO_EXPR, this-> test-> token.getLocus (), void_type_node, test_label);
	Ymir::Tree goto_end = Ymir::buildTree (GOTO_EXPR, this-> test-> token.getLocus (), void_type_node, end_label);
	Ymir::Tree goto_begin = Ymir::buildTree (GOTO_EXPR, this-> test-> token.getLocus (), void_type_node, begin_label);
	
	Ymir::Tree test_expr = Ymir::buildTree (COND_EXPR, this-> test-> token.getLocus (), void_type_node, bool_expr, goto_begin, goto_end);
	Ymir::Tree begin_label_expr = Ymir::buildTree (LABEL_EXPR, this-> block-> token.getLocus (), void_type_node, begin_label);
	list.append (goto_test);
	list.append (begin_label_expr);
	Ymir::Tree begin_part = this-> block-> toGeneric ();
	list.append (begin_part);
	list.append (goto_test);

	Ymir::Tree test_label_expr = Ymir::buildTree (LABEL_EXPR, this-> test-> token.getLocus (), void_type_node, test_label);
	list.append (test_label_expr);
	list.append (test_expr);

	Ymir::Tree end_expr = Ymir::buildTree (LABEL_EXPR, this-> test-> token.getLocus (), void_type_node, end_label);	
	list.append (end_expr);
	
	return list.getTree ();	
    }    

    Ymir::Tree IArrayAlloc::toGeneric () {
	Ymir::TreeStmtList list;
	ArrayInfo info = this-> info-> type-> to <IArrayInfo> ();
	Ymir::Tree innerType = info-> content ()-> toGeneric ();
	Ymir::Tree array_type = info-> toGeneric ();
	auto lenr = this-> size-> toGeneric ();
	std::vector <tree> args = {fold_convert (build_pointer_type (void_type_node), info-> content ()-> getInitFnPtr ().getTree ()), lenr.getTree ()};
	
	auto ptrr = build_call_array (
	    build_pointer_type (void_type_node),
	    semantic::InternalFunction::getYNewArray ().getTree (),
	    args.size (), args.data ()
	);				      

	Ymir::Tree aux = Ymir::makeAuxVar (this-> token.getLocus (),
					   ISymbol::getLastTmp (),
					   array_type
	);
	
	Ymir::Tree lenl = Ymir::getField (this-> token.getLocus (), aux, "len");
	Ymir::Tree ptrl = Ymir::getField (this-> token.getLocus (), aux, "ptr");
	
	list.append (Ymir::buildTree (
	    MODIFY_EXPR, this-> token.getLocus (),
	    void_type_node,
	    lenl, fold_convert (long_unsigned_type_node, lenr.getTree ())
	));

	list.append (Ymir::buildTree (
	    MODIFY_EXPR, this-> token.getLocus (),
	    void_type_node,
	    ptrl,
	    fold_convert (build_pointer_type (innerType.getTree ()), ptrr)
	));

	Ymir::getStackStmtList ().back ().append (list.getTree ());
	return aux;
    }
    
    Ymir::Tree IConstTuple::toGeneric () {
	location_t loc = this-> token.getLocus ();
	auto tuple_type = this-> info-> type-> toGeneric ();
	std::vector <InfoType> type_inner = this-> info-> type-> to <ITupleInfo> ()-> getParams ();
	Ymir::TreeStmtList list;
	auto aux = Ymir::makeAuxVar (loc, ISymbol::getLastTmp (), tuple_type);
	for (auto it : Ymir::r (0, this-> params.size ())) {
	    auto field = Ymir::getField (loc, aux, it);
	    list.append (this-> casters [it]-> buildBinaryOp (
		this-> params [it]-> token,
		this-> casters [it],
		new ITreeExpression (this-> token, type_inner [it], field),
		this-> params [it]
	    ));
	}
	Ymir::getStackStmtList ().back ().append (list.getTree ());
	return aux;
    }
    
    Ymir::Tree IExpand::toGeneric () {
	return Ymir::Tree ();
    }

}




