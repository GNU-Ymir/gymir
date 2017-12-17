#include <ymir/ast/_.hh>
#include <ymir/semantic/tree/Tree.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/semantic/types/_.hh>
#include <ymir/errors/Error.hh>
#include <ymir/semantic/pack/FinalFrame.hh>
#include "print-tree.h"

namespace syntax {
    using namespace semantic;
    
    Ymir::Tree IInstruction::toGeneric () {
	Ymir::Error::assert ((std::string ("TODO ") + this-> getId ()).c_str  ());
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
	return Ymir::leaveBlock ().bind_expr;
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
    
    Ymir::Tree IVar::toGeneric () {
	return this-> info-> treeDecl ();
    }

    Ymir::Tree IBinary::toGeneric () {
	return this-> info-> type-> buildBinaryOp (
	    this-> token,
	    this-> left,
	    this-> right
	);
    }

    Ymir::Tree IAccess::toGeneric () {
	if (this-> info-> type-> binopFoo) {
	    return this-> info-> type-> buildBinaryOp (
		this-> token,
		this-> left,
		this-> params-> getParams () [0]
	    );
	} else {
	    return this-> info-> type-> buildMultOp (
		this-> token,
		this-> left,
		this-> params
	    );
	}
    }
    
    std::vector <tree> IParamList::toGenericParams (std::vector <semantic::InfoType> treat) {
	std::vector <tree> params (this-> params.size ());
	for (int i = 0 ; i < this-> params.size () ; i++) {
	    Ymir::Tree elist = this-> params [i]-> toGeneric ();
	    if (treat [i]) {
		//for (long nb = treat [i
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
	
	for (int i = 0 ; i < this-> params.size () ; i++) {
	    Ymir::Tree ref = Ymir::getArrayRef (this-> token.getLocus (), aux, innerType, i);
	    auto left = new ITreeExpression (this-> token, ref);
	    left-> info = new ISymbol (this-> token, info-> content ());
	    list.append (this-> casters [i]-> buildBinaryOp (this-> token, left, this-> params [i]));
	}

	Ymir::getStackStmtList ().back ().append (list.getTree ());
	return aux;
    }

    Ymir::Tree IDot::toGeneric () {
	if (this-> right-> is<IVar> ()) {
	    return this-> info-> type-> buildUnaryOp (
		this-> token,
		this-> left
	    );
	} else {
	    return this-> info-> type-> buildBinaryOp (
		this-> token,
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

    
}




