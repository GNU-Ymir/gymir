#include <ymir/syntax/Keys.hh>
#include <ymir/ast/_.hh>
#include <ymir/semantic/tree/Tree.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/semantic/types/_.hh>
#include <ymir/errors/Error.hh>
#include <ymir/semantic/pack/FinalFrame.hh>
#include <ymir/semantic/pack/InternalFunction.hh>
#include <ymir/semantic/utils/StringUtils.hh>
#include <ymir/semantic/utils/ArrayUtils.hh>
#include <ymir/semantic/utils/FunctionUtils.hh>
#include <ymir/semantic/object/AggregateInfo.hh>
#include "print-tree.h"
#include <ymir/semantic/value/_.hh>

namespace syntax {
    using namespace semantic;
    
    Ymir::Tree IInstruction::toGeneric () {
	this-> print (0);
	Ymir::Error::assert ((std::string ("TODO generic") + this-> getIds ().back ()).c_str  ());
	return NULL;
    }

    Ymir::Tree INone::toGeneric () {
	return Ymir::Tree ();
    }
    
    Ymir::Tree IBlock::toGenericValue () {
	auto expr = this-> _value;	
	this-> _value = NULL;
	this-> _insts.push_back (expr);
	InfoType type;
	Ymir::Tree res;
	auto list = toGenericExpr (type, res);
	Ymir::getStackStmtList ().back ().append (list);
	return res;
    }

    Ymir::Tree IBlock::toGeneric () {
	if (this-> _value != NULL) {
	    return this-> toGenericValue ();
	}
	
	Ymir::enterBlock ();
	IBlock::__currentBlock__.push_front (this);	
	for (auto it : this-> _inlines) {
	    if (!it-> info-> type ()-> is <IUndefInfo> ())
		Ymir::getStackStmtList ().back ().append (it-> toGeneric ());
	}
	
	for (auto it : this-> _insts) {
	    auto inst = it-> toGeneric ();
	    Ymir::getStackStmtList ().back ().append (inst);    
	}
	
	IBlock::__currentBlock__.pop_front ();
	auto body = Ymir::leaveBlock ();
	if (this-> _finally.size () == 0)
	    return body.bind_expr;

	if (this-> _finally.size () == 1) {
	    auto finally = this-> _finally [0]-> toGeneric ();
	    return build2 (TRY_FINALLY_EXPR, void_type_node, body.bind_expr.getTree (), finally.getTree ());
	} else {
	    Ymir::enterBlock ();
	    for (auto it : this-> _finally) {
		auto inst = it-> toGeneric ();
		Ymir::getStackStmtList ().back ().append (inst);
	    }
	    auto finally = Ymir::leaveBlock ();
	    return build2 (TRY_FINALLY_EXPR, void_type_node, body.bind_expr.getTree (), finally.bind_expr.getTree ());
	}      
    }
    
    Ymir::Tree IBlock::toGenericExpr (InfoType & type, Ymir::Tree & expr) {
	auto last = this-> _insts.back ()-> to <IExpression> ();
	this-> _insts.pop_back ();
	if (!last-> info-> value ()) {
	    auto res = Ymir::makeAuxVar (this-> token.getLocus (), ISymbol::getLastTmp (), last-> info-> type ()-> toGeneric ());	
	    Ymir::enterBlock ();
	    IBlock::__currentBlock__.push_front (this);
	    for (auto it : this-> _insts) {
		it-> father () = this;
		auto inst = it-> toGeneric ();
		Ymir::getStackStmtList ().back ().append (inst);    
	    }
	    type = last-> info-> type ();
	    expr = last-> toGeneric ();

	    Ymir::getStackStmtList ().back ().append (
		buildTree (MODIFY_EXPR,
			   this-> token.getLocus (),
			   expr.getType (),
			   res, expr)
	    );
	
	    expr = res;
	} else {
	    Ymir::enterBlock ();
	    IBlock::__currentBlock__.push_front (this);
	    for (auto it : this-> _insts) {
		it-> father () = this;
		auto inst = it-> toGeneric ();
		Ymir::getStackStmtList ().back ().append (inst);    
	    }
	    type = last-> info-> type ();
	    expr = last-> toGeneric ();	    
	}

	IBlock::__currentBlock__.pop_front ();
	auto body = Ymir::leaveBlock ();
	if (this-> _finally.size () == 0)
	    return body.bind_expr;

	if (this-> _finally.size () == 1) {
	    auto finally = this-> _finally [0]-> toGeneric ();
	    return build2 (TRY_FINALLY_EXPR, void_type_node, body.bind_expr.getTree (), finally.getTree ());
	} else {
	    Ymir::enterBlock ();
	    for (auto it : this-> _finally) {
		auto inst = it-> toGeneric ();
		Ymir::getStackStmtList ().back ().append (inst);
	    }
	    auto finally = Ymir::leaveBlock ();
	    return build2 (TRY_FINALLY_EXPR, void_type_node, body.bind_expr.getTree (), finally.bind_expr.getTree ());
	}
    }
        
    Ymir::Tree IVarDecl::toGeneric () {
	Ymir::TreeStmtList list;
	for (int i = 0 ; i < (int) this-> decls.size () ; i++) {
	    auto var = this-> decls [i];
	    auto aff = this-> insts [i];
	    if (!var-> info-> isImmutable ()) {
		auto type_tree = var-> info-> type ()-> toGeneric ();
		auto inner = type_tree;
		
		if (var-> info-> isClosured ()) 
		    type_tree = build_pointer_type (type_tree.getTree ());
		
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

		if (var-> info-> isClosured ()) {
		    auto fn = InternalFunction::getMalloc ();
		    auto byte_len = TYPE_SIZE_UNIT (inner.getTree ());
		    auto alloc = build_call_array_loc (this-> token.getLocus (),
						       type_tree.getTree (),
						       fn.getTree (), 1, &byte_len);
		    
		    list.append (buildTree (
			MODIFY_EXPR,
			this-> token.getLocus (),
			type_tree, decl, alloc
		    ));
		}
		
		if (aff != NULL) {
		    if (aff-> info)
			aff-> info-> value () = NULL;
		    list.append (aff-> toGeneric ());
		}

		if (var-> info-> type ()-> is <IAggregateInfo> ()) {
		    if (auto frame = var-> info-> type ()-> to <IAggregateInfo> ()-> getDestructor ()) {
			auto proto = frame-> validate ();
			std::vector <tree> params = {getAddr (decl).getTree ()};
			auto destr = build_call_array_loc (this-> token.getLocus (), void_type_node, proto-> toGeneric ().getTree (), 1, params.data ());
			auto block = new (Z0) IBlock (this-> token, {}, {new (Z0) ITreeExpression (this-> token, NULL, destr)});
			IBlock::getCurrentBlock ()-> addFinally (block);
		    }
		}
	    }
	}
	
	for (auto it : Ymir::r (0, this-> statics.size ())) {
	    if (this-> statics [it]-> info-> treeDecl ().isNull ()) {
		Ymir::declareGlobal (this-> statics [it]-> info, this-> staticExprs [it]);
	    }
	}
	
	return list.getTree ();
    }

    Ymir::Tree IFixed::toGeneric () {
	switch (this-> _type) {
	case FixedConst::BYTE : return build_int_cst_type (signed_char_type_node, this-> _value);
	case FixedConst::SHORT : return build_int_cst_type (short_integer_type_node, this-> _value);
	case FixedConst::INT : return build_int_cst_type (integer_type_node, this-> _value);
	case FixedConst::LONG : return build_int_cst_type (long_integer_type_node, this-> _value);		

	case FixedConst::UBYTE : return build_int_cst_type (unsigned_char_type_node, this-> _uvalue);
	case FixedConst::USHORT : return build_int_cst_type (short_unsigned_type_node, this-> _uvalue);		
	case FixedConst::UINT : return build_int_cst_type (unsigned_type_node, this-> _uvalue);
	case FixedConst::ULONG : return build_int_cst_type (long_unsigned_type_node, this-> _uvalue);
	}
	
	return Ymir::Tree ();
    }

    Ymir::Tree IChar::toGeneric () {
	return build_int_cst_type (unsigned_char_type_node, this-> _code);
    }
    
    Ymir::Tree IBool::toGeneric () {
	return build_int_cst_type (boolean_type_node, this-> _value);
    }

    Ymir::Tree IFloat::toGeneric () {
	REAL_VALUE_TYPE real_value ;
	auto str = this-> getValue ();
	if (str == "INF") str = "Inf";
	else if (str == "NAN") str = "QNaN";
	Ymir::Tree type = this-> info-> type ()-> toGeneric ();
	real_from_string (&real_value, str.c_str ());
	return build_real (type.getTree (), real_value);
    }
    
    Ymir::Tree IVar::lastInfoDecl () {
	if (this-> _lastInfo-> type ()-> unopFoo) {
	    return this-> _lastInfo-> type ()-> buildUnaryOp (
		this-> token,
		this-> _lastInfo-> type (),
		this
	    );	    
	} else return this-> _lastInfo-> treeDecl ();
    }

    Ymir::Tree IVar::toGeneric () {
	if (this-> info-> value ())
	    return this-> info-> value ()-> toYmir (this-> info)-> toGeneric ();
	
	if (this-> info-> type ()-> unopFoo) {
	    return this-> info-> type ()-> buildUnaryOp (
		this-> token,
		this-> info-> type (),
		this
	    );	    
	} else {
	    if (this-> _fromClosure) {
		auto closure = IFinalFrame::getCurrentClosure ();
		auto field = Ymir::getField (this-> token.getLocus (),
					     closure,
					     this-> token.getStr ());
		return field;
	    } else {
		auto ret = this-> info-> treeDecl ();
		if (this-> info-> isInline ()) {
		    if (ret.isNull ()) {
			Ymir::Tree decl = build_decl (
			    this-> token.getLocus (),
			    VAR_DECL,
			    get_identifier (this-> token.getStr ().c_str ()),			    
			    this-> info-> type ()-> toGeneric ().getTree ());
			DECL_CONTEXT (decl.getTree ()) = IFinalFrame::currentFrame ().getTree ();
			this-> info-> treeDecl (decl);
			Ymir::getStackVarDeclChain ().back ().append (decl);
			return Ymir::compoundExpr (this-> token.getLocus (), buildTree (DECL_EXPR, this-> token.getLocus (), void_type_node, decl), decl);
		    } else return ret;
		} else if (this-> info-> isClosured ()) {
		    return Ymir::getPointerUnref (
			this-> token.getLocus (),
			ret,
			this-> info-> type ()-> toGeneric (),
			0
		    );
		} else {
		    return ret;
		}
	    }
	}
    }

    Ymir::Tree IBinary::toGeneric () {
	if (this-> info-> isImmutable ()) {
	    auto ret = this-> info-> value ()-> toYmir (this-> info)-> toGeneric ();
	    return ret;
	}
	
	return this-> info-> type ()-> buildBinaryOp (
	    this-> token,
	    this-> info-> type (),
	    this-> _left,
	    this-> _right
	);
    }

    Ymir::Tree ICast::toGeneric () {
	if (this-> info-> type ()-> unopFoo) {
	    return this-> info-> type ()-> buildUnaryOp (
		this-> token,
		this-> info-> type (),
		this-> _expr
	    );
	} else if (this-> info-> type ()-> binopFoo) {
	    return this-> info-> type ()-> buildBinaryOp (
		this-> token,
		this-> info-> type (),
		this-> _expr,
		new (Z0)  ITreeExpression (this-> token, this-> info-> type (), Ymir::Tree ())
	    );
	} else {
	    return this-> _expr-> toGeneric ();
	}
    }
    
    Ymir::Tree IUnary::toGeneric () {
	if (this-> info-> isImmutable ()) {
	    auto ret = this-> info-> value ()-> toYmir (this-> info)-> toGeneric ();
	    return ret;
	}
	
	if (this-> info-> type ()-> unopFoo) {
	    return this-> info-> type ()-> buildUnaryOp (
		this-> token,
		this-> info-> type (),
		this-> elem
	    );
	} else {
	    return this-> info-> type ()-> buildBinaryOp (
		this-> token,
		this-> info-> type (),
		this-> elem,
		new (Z0)  ITreeExpression (this-> token, this-> info-> type (), Ymir::Tree ())
	    );
	}
    }
    
    Ymir::Tree IAccess::toGeneric () {
	if (this-> info-> isImmutable ()) {
	    auto ret = this-> info-> value ()-> toYmir (this-> info)-> toGeneric ();
	    return ret;
	}

	std::vector <tree> args = this-> _params-> toGenericParams (this-> _treats);
	if (this-> info-> type ()-> binopFoo) {
	    return this-> info-> type ()-> buildBinaryOp (
		this-> token,
		this-> info-> type (),
		this-> _left,
		new (Z0)  ITreeExpression (this-> _params-> getParams ()[0]-> token, this-> _params-> getParams () [0]-> info-> type (), args [0])
	    );
	} else {
	    IParamList params (this-> _params-> token, {});
	    for (auto it : Ymir::r (0, args.size ())) {
		params.params ().push_back (new (Z0)  ITreeExpression (this-> _params-> getParams () [it]-> token,
									  this-> _params-> getParams () [it]-> info-> type (),
									  args [it]));
	    }
	    return this-> info-> type ()-> buildMultOp (
		this-> token,
		this-> info-> type (),
		this-> _left,
		&params,
		NULL
	    );
	}
    }
    
    std::vector <tree> IParamList::toGenericParams (const std::vector <semantic::InfoType>& treat) {
	std::vector <tree> params (this-> _params.size ());
	for (uint i = 0 ; i < this-> _params.size () ; i++) {
	    Ymir::Tree elist;
	    if (treat [i] && treat [i]-> binopFoo) {
		elist = treat [i]-> buildCastOp (
		    this-> _params [i]-> token,
		    treat [i],
		    this-> _params [i],
		    new (Z0)  ITreeExpression (this-> _params [i]-> token, treat [i], Ymir::Tree ())
		);
	    } else {
		elist = this-> _params [i]-> toGeneric ();
	    }
	    params [i] = elist.getTree ();
	}
	return params;
    }
    
    Ymir::Tree IStructCst::toGeneric () {
	this-> params-> treats () = this-> score-> treat;
	return this-> score-> ret-> buildMultOp (
	    this-> token,
	    this-> score-> ret,
	    this-> left,
	    this-> params,
	    NULL
	);	
    }
    
    Ymir::Tree IPar::callInline (std::vector <tree> args) {
	auto frame = this-> _score-> proto-> attached ();
	return frame-> callInline (this-> _left-> token, args);
    }

    Ymir::Tree IPar::toGeneric () {
	if (this-> info-> isImmutable ()) {
	    auto ret = this-> info-> value ()-> toYmir (this-> info)-> toGeneric ();
	    return ret;
	}

	if (this-> _score-> ret-> multFoo) {
	    this-> _params-> treats () = this-> _score-> treat;
	    return this-> _score-> ret-> buildMultOp (
		this-> token,
		this-> _score-> ret,
		this-> _left,
		this-> _params,
		this-> _score
	    );
	} else {
	    std::vector <tree> args = this-> _params-> toGenericParams (this-> _score-> treat);
	    if (this-> _score-> proto-> isCVariadic ()) {
		for (auto & it : args) {
		    it = Ymir::promote (it);
		}
	    } else if (this-> _score-> proto-> closure ().size () != 0) {
		auto closureVar = semantic::FunctionUtils::createClosureVar (this-> token.getLocus (), this-> _score);
		args.insert (args.begin (), closureVar.getTree ());
	    } else if (this-> _score-> proto-> has (Keys::INLINE)) {
		return this-> callInline (args);
	    } else if (this-> _left-> info-> type ()-> is <IFunctionInfo> () &&
		       this-> _left-> info-> type ()-> to <IFunctionInfo> ()-> isConstr ()) {

		Ymir::Tree ltree;
		bool obj = false;
		if (auto dcol = this-> _left-> to <IDColon> ()) {
		    if (dcol-> getLeft ()-> info-> type ()-> is <IAggregateInfo> ()) {
			obj = true;
			ltree = dcol-> getLeft ()-> toGeneric ();
		    }
		}
		    
		if (!obj) {
		    ltree = Ymir::makeAuxVar (this-> token.getLocus (), ISymbol::getLastTmp (), this-> _score-> ret-> toGeneric ());
		}
		
		args.insert (args.begin (), getAddr (ltree).getTree ());
		Ymir::TreeStmtList list;
		Ymir::Tree fn = this-> _score-> proto-> toGeneric ();

		if (!obj) {
		    auto vtable = Ymir::getAddr (this-> _score-> ret-> to <IAggregateInfo> ()-> getVtable ());
		    auto vfield = Ymir::getField (this-> token.getLocus (), ltree, Keys::VTABLE_FIELD);
		    list.append (
			Ymir::buildTree (MODIFY_EXPR,
					 this-> token.getLocus (),
					 void_type_node,
					 vfield,
					 convert (vfield.getType ().getTree (), vtable.getTree ())
			)
		    );
		}
		
		list.append (build_call_array_loc (this-> token.getLocus (),
						   void_type_node,
						   fn.getTree (),
						   args.size (),
						   args.data ()
		));

		if (!obj) {
		    if (auto frame = this-> _score-> ret-> to<IAggregateInfo> ()-> getDestructor ()) {
			auto proto = frame-> validate ();
			std::vector <tree> params = {getAddr (ltree).getTree ()};
			auto destr = build_call_array_loc (this-> token.getLocus (), void_type_node, proto-> toGeneric ().getTree (), 1, params.data ());
			auto block = new (Z0) IBlock (this-> token, {}, {new (Z0) ITreeExpression (this-> token, NULL, destr)});
			IBlock::getCurrentBlock ()-> addFinally (block);
		    }
		}
		return Ymir::compoundExpr (this-> token.getLocus (), list.getTree (), ltree);
	    } 
	    
	    Ymir::Tree fn = this-> _score-> proto-> toGeneric ();
	    return build_call_array_loc (this-> token.getLocus (),
					 this-> _score-> ret-> toGeneric ().getTree (),
					 fn.getTree (),
					 args.size (),
					 args.data ()
	    );	    
	}
    }
    
    Ymir::Tree IString::toGeneric () {
	return build_string_literal (this-> _content.length () + 1, this-> _content.c_str ());       
    }

    Ymir::Tree IConstArray::toGeneric () {
	Ymir::TreeStmtList list;
	if (this-> params.size () != 0) {
	    ArrayInfo info = this-> info-> type ()-> to<IArrayInfo> ();	
	    Ymir::Tree innerType = info-> content ()-> toGeneric ();
	    auto intExpr = new (Z0)  IFixed (this-> token, FixedConst::ULONG);
	    intExpr-> setUValue (this-> params.size () - 1);
	    auto lenExpr = intExpr-> expression ();			
	    auto len = lenExpr-> toGeneric ();
	    intExpr-> setUValue (0);
	    auto begin = intExpr-> toGeneric ();
	
	    Ymir::Tree range_type = build_range_type (integer_type_node, fold (begin.getTree ()), fold (len.getTree ()));
	    Ymir::Tree array_type = build_array_type (innerType.getTree (), range_type.getTree ());
	
	    Ymir::Tree aux = Ymir::makeAuxVar (this-> token.getLocus (),
					       ISymbol::getLastTmp (),
					       array_type
	    );

	    for (uint i = 0 ; i < this-> params.size () ; i++) {
		Ymir::Tree ref = Ymir::getArrayRef (this-> token.getLocus (), aux, innerType, i);
		auto left = new (Z0)  ITreeExpression (this-> token, info-> content (), ref);
		auto right = this-> casters [i]-> buildCastOp (this-> token, this-> casters [i], this-> params [i], left);
		if (ref.getType () == right.getType ()) {
		    list.append (Ymir::buildTree (MODIFY_EXPR, this-> token.getLocus (),
						  void_type_node,
						  ref, right)
		    );
		} else {
		    auto ptrl = Ymir::getAddr (this-> token.getLocus (), ref).getTree ();
		    auto ptrr = Ymir::getAddr (this-> token.getLocus (), right).getTree ();
		    tree tmemcpy = builtin_decl_explicit (BUILT_IN_MEMCPY);
		    tree size = TYPE_SIZE_UNIT (ref.getType ().getTree ());
		    auto res = build_call_expr (tmemcpy, 3, ptrl, ptrr, size);
		    list.append (res);
		}
	    }

	    return Ymir::compoundExpr (this-> token.getLocus (), list.getTree (), aux);
	} else {
	    auto intExpr = new (Z0)  IFixed (this-> token, FixedConst::ULONG);
	    intExpr-> setUValue (0);
	    auto lenExpr = intExpr-> expression ();			
	    auto zero = lenExpr-> toGeneric ();
	    
	    auto loc = this-> token.getLocus ();
	    auto aux = makeAuxVar (loc, ISymbol::getLastTmp (), this-> info-> type ()-> toGeneric ());
	    auto len = getField (loc, aux, "len");
	    auto ptr = getField (loc, aux, "ptr");

	    list.append (buildTree (MODIFY_EXPR, loc, void_type_node, len, zero));
	    list.append (buildTree (MODIFY_EXPR, loc, void_type_node, ptr, zero));
	    return Ymir::compoundExpr (loc, list.getTree (), aux);
	}
    }

    Ymir::Tree IDot::toGeneric () {
	if (this-> info-> isImmutable ()) {
	    auto ret = this-> info-> value ()-> toYmir (this-> info)-> toGeneric ();
	    return ret;
	}

	return this-> info-> type ()-> buildBinaryOp (
	    this-> token,
	    this-> info-> type (),
	    this-> _left,
	    this-> _right
	);
    }    

    Ymir::Tree IDColon::toGeneric () {
	if (this-> info-> isImmutable ()) {
	    auto ret = this-> info-> value ()-> toYmir (this-> info)-> toGeneric ();
	    return ret;
	}
	
	return this-> info-> type ()-> buildUnaryOp (
	    this-> token,
	    this-> info-> type (),
	    this-> _left
	);
    }    
    
    Ymir::Tree ITreeExpression::toGeneric () {
	return this-> _content;
    }

    Ymir::Tree ISemanticConst::toGeneric () {
	return this-> info-> type ()-> buildUnaryOp (
	    this-> token,
	    this-> info-> type (),
	    new (Z0) ITreeExpression (this-> token, this-> _auxType, Ymir::Tree ())
	);
    }
    
    Ymir::Tree INull::toGeneric () {
	return build_int_cst_type (long_unsigned_type_node, 0);
    }

    Ymir::Tree IIf::toGeneric () {
	if (this-> _test != NULL) {
	    Ymir::Tree bool_expr = this-> _test-> toGeneric ();
	    Ymir::Tree thenLabel = Ymir::makeLabel (this-> token.getLocus (), "then");
	    Ymir::Tree endLabel  = Ymir::makeLabel (this-> token.getLocus (), "end_if");
	    Ymir::Tree goto_then = Ymir::buildTree (GOTO_EXPR, this-> _test-> token.getLocus (), void_type_node, thenLabel);
	    Ymir::Tree goto_end = Ymir::buildTree (GOTO_EXPR, this-> _test-> token.getLocus (), void_type_node, endLabel);
	    Ymir::Tree goto_else, elseLabel;
	    
	    if (this-> _else) {
		elseLabel = Ymir::makeLabel (this-> token.getLocus (), "else");
		goto_else = Ymir::buildTree (GOTO_EXPR, this-> _test-> token.getLocus (), void_type_node, elseLabel);
	    } else {
		goto_else = goto_end;
	    }
	    
	    Ymir::TreeStmtList list;
	    Ymir::Tree cond_expr = Ymir::buildTree (COND_EXPR, this-> _test-> token.getLocus (), void_type_node, bool_expr, goto_then, goto_else);
	    list.append (cond_expr);
	    
	    Ymir::Tree then_label_expr = Ymir::buildTree (LABEL_EXPR, this-> _block-> token.getLocus (), void_type_node, thenLabel);

	    list.append (then_label_expr);
	    Ymir::Tree then_part = this-> _block-> toGeneric ();
	    list.append (then_part);
	    list.append (goto_end);
	    
	    if (this-> _else) {		
		Ymir::Tree else_label_expr = Ymir::buildTree (LABEL_EXPR, this-> _else-> token.getLocus (), void_type_node, elseLabel);
		list.append (else_label_expr);
		Ymir::Tree else_part = this-> _else-> toGeneric ();
		list.append (else_part);
		list.append (goto_end);
	    }

	    Ymir::Tree endif_label_expr = Ymir::buildTree (LABEL_EXPR, this-> token.getLocus (), void_type_node, endLabel);
	    list.append (endif_label_expr);

	    return  list.getTree ();
	} else {
	    return this-> _block-> toGeneric ();
	}
    }

    Ymir::Tree IFor::toGeneric () {
	Ymir::TreeStmtList list;
	for (int i = 0; i < (int) this-> _var.size () ; i++) {
	    auto var = this-> _var [i];
	    auto type_tree = var-> info-> type ()-> toGeneric ();
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
	}
	Ymir::Tree end_label = Ymir::makeLabel (this-> token.getLocus (), "end");
	Ymir::getLoopLabels ().push_back (end_label);
	if (!this-> _id.isEof ())
	    Ymir::getLoopLabelsNamed () [this-> _id.getStr ()] = end_label;
	
	list.append (this-> _ret-> buildApplyOp (this-> token, this-> _ret, this-> _var, this-> _block, this-> _iter));
	Ymir::Tree end_expr = Ymir::buildTree (LABEL_EXPR, this-> token.getLocus (), void_type_node, end_label);	
	list.append (end_expr);
	Ymir::getLoopLabels ().pop_back ();
	if (!this-> _id.isEof ())
	    Ymir::getLoopLabelsNamed ().erase (this-> _id.getStr ());

	return list.getTree ();
    }
    
    Ymir::Tree IWhile::toGeneric () {
	Ymir::TreeStmtList list;
	Ymir::Tree bool_expr = this-> test-> toGeneric ();
	Ymir::Tree test_label = Ymir::makeLabel (this-> token.getLocus (), "test");
	Ymir::Tree begin_label = Ymir::makeLabel (this-> token.getLocus (), "begin");
	Ymir::Tree end_label = Ymir::makeLabel (this-> token.getLocus (), "end");

	Ymir::getLoopLabels ().push_back (end_label);	
	if (!this-> name.isEof ())
	    Ymir::getLoopLabelsNamed () [this-> name.getStr ()] = end_label;
	
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
	Ymir::getLoopLabels ().pop_back ();
	if (!this-> name.isEof ())
	    Ymir::getLoopLabelsNamed ().erase (this-> name.getStr ());
	
	return list.getTree ();	
    }    

    Ymir::Tree IBreak::toGeneric () {
	Ymir::Tree label;
	if (this-> _ident.isEof ()) {
	    label = Ymir::getLoopLabels ().back ();
	} else {
	    label = Ymir::getLoopLabelsNamed () [this-> _ident.getStr ()];
	}
	return Ymir::buildTree (GOTO_EXPR, this-> token.getLocus (), void_type_node, label);
    }
    
    Ymir::Tree IArrayAlloc::toGeneric () {
	ArrayInfo info = this-> info-> type ()-> to <IArrayInfo> ();
	Ymir::Tree innerType = info-> content ()-> toGeneric ();	
	Ymir::Tree array_type = info-> toGeneric ();
	
	if (array_type.getTreeCode () == RECORD_TYPE) {
	    return dynamicGeneric (info, innerType, array_type);
	} else {
	    return staticGeneric (info, innerType, array_type);
	}
    }

    Ymir::Tree IArrayAlloc::staticGeneric (semantic::ArrayInfo, Ymir::Tree, Ymir::Tree array_type) {
	Ymir::TreeStmtList list;
	auto loc = this-> token.getLocus ();
	auto aux = Ymir::makeAuxVar (loc, ISymbol::getLastTmp (), array_type);
	auto addr = Ymir::getAddr (loc, aux);
	tree memsetArgs [] = {addr.getTree (),
			      build_int_cst_type (long_unsigned_type_node, 0),
			      TYPE_SIZE_UNIT (aux.getType ().getTree ())};
	
	return Ymir::compoundExpr (loc,
				   build_call_array_loc (loc, void_type_node, InternalFunction::getYMemset ().getTree (), 3, memsetArgs),
				   aux);
    }
    
    Ymir::Tree IArrayAlloc::dynamicGeneric (semantic::ArrayInfo, Ymir::Tree innerType, Ymir::Tree array_type) {
	Ymir::TreeStmtList list;	
	auto lenr = this-> _size-> toGeneric ();
	std::vector <tree> args = {convert (long_unsigned_type_node, TYPE_SIZE_UNIT (innerType.getTree ())), lenr.getTree ()};
	
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

	return Ymir::compoundExpr (this-> token.getLocus (), list.getTree (), aux);
    }
    
    Ymir::Tree IConstTuple::toGeneric () {
	location_t loc = this-> token.getLocus ();
	auto tuple_type = this-> info-> type ()-> toGeneric ();
	std::vector <InfoType> type_inner = this-> info-> type ()-> to <ITupleInfo> ()-> getParams ();
	Ymir::TreeStmtList list;
	auto aux = Ymir::makeAuxVar (loc, ISymbol::getLastTmp (), tuple_type);
	for (auto it : Ymir::r (0, this-> params.size ())) {
	    auto field = Ymir::getField (loc, aux, it);
	    auto right = this-> casters [it]-> buildBinaryOp (
		this-> params [it]-> token,
		type_inner [it],
		this-> params [it],
		new (Z0)  ITreeExpression (this-> token, type_inner [it], Ymir::Tree ())
	    );
	    if (right.getType () == field.getType ()) {
		list.append (
		    buildTree (
			MODIFY_EXPR, loc, void_type_node, field, right			
		    )
		);
	    } else {
		auto ptrl = Ymir::getAddr (loc, field).getTree ();
		auto ptrr = Ymir::getAddr (loc, right).getTree ();
		tree tmemcopy = builtin_decl_explicit (BUILT_IN_MEMCPY);
		tree size = TYPE_SIZE_UNIT (field.getType ().getTree ());	    
		auto result = build_call_expr_loc (loc, tmemcopy, 3, ptrl, ptrr, size);
		list.append (result);
	    }	    
	}
	
	return Ymir::compoundExpr (loc, list.getTree (), aux);
    }
    
    Ymir::Tree IExpand::toGeneric () {
	location_t loc = this-> token.getLocus ();
	Ymir::Tree elemTree;
	auto it = Ymir::getAssocTree ().find ((void*) this-> _expr);
	if (it == Ymir::getAssocTree ().end ()) {
	    elemTree = this-> _expr-> toGeneric ();
	    Ymir::getAssocTree () [(void*) this-> _expr] = elemTree;
	} else {
	    elemTree = it-> second;
	}

	if (auto ref = this-> _expr-> info-> type ()-> to <IRefInfo> ()) {
	    auto inner = ref-> content ()-> toGeneric ();
	    elemTree = getPointerUnref (loc, elemTree, inner, 0);
	}
	
	return getField (loc, elemTree, this-> _it);
    }

    Ymir::Tree IReturn::toGeneric () {
	Ymir::Tree res;
	Ymir::Tree tlvalue;
	if (IFinalFrame::endLabel ().isNull ())
	    tlvalue = DECL_RESULT (IFinalFrame::currentFrame ().getTree ());
	else tlvalue = IFinalFrame::isInlining ();
	
	if (this-> _elem != NULL) {
	    this-> _caster-> isConst (false);
	    if (this-> _caster-> unopFoo) {
		res = this-> _caster-> buildUnaryOp (
		    this-> token,
		    this-> _caster,
		    this-> _elem
		);
	    } else {
		res = this-> _caster-> buildCastOp (
		    this-> token,
		    this-> _caster,
		    this-> _elem,
		    new (Z0)  ITreeExpression (this-> token, this-> _caster, Ymir::Tree ())
		);
	    }
	}
	
	auto set_result = this-> _elem != NULL ?
	    buildTree (MODIFY_EXPR, this-> token.getLocus (), void_type_node, tlvalue, res) :
	    Ymir::Tree ();
	if (IFinalFrame::endLabel ().isNull ()) 
	    return Ymir::buildTree (RETURN_EXPR, this-> token.getLocus (), void_type_node, set_result);
	else {
	    Ymir::TreeStmtList list;
	    list.append (set_result);
	    list.append (Ymir::buildTree (GOTO_EXPR, this-> token.getLocus (), void_type_node, IFinalFrame::endLabel ()));
	    return list.getTree ();
	}
    }

    Ymir::Tree IFuncPtr::toGeneric () {
	return build_int_cst_type (long_unsigned_type_node, 0);	   
    }

    Ymir::Tree ITupleDest::toGeneric () {
	Ymir::TreeStmtList list;
	for (int i = 0 ; i < (int) this-> decls.size () ; i++) {
	    auto var = this-> decls [i];
	    auto aff = this-> insts [i];
	    auto type_tree = var-> info-> type ()-> toGeneric ();
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
		aff-> info-> value () = NULL;
	    	list.append (aff-> toGeneric ());
	    }
	}

	
	return list.getTree ();
    }    

    Ymir::Tree IIs::toGeneric () {
	if (this-> info-> isImmutable ()) {
	    auto ret = this-> info-> value ()-> toYmir (this-> info)-> toGeneric ();
	    return ret;
	} else {
	    Ymir::Error::assert ("");
	    return Ymir::Tree ();
	}
    }

    Ymir::Tree IMatch::declareVars (std::vector <Var> vars, std::vector <Expression> casters) {
	Ymir::TreeStmtList list;
	for (auto it : Ymir::r (0, vars.size ())) {
	    auto var = vars [it];
	    auto aff = casters [it];
	    auto type_tree = var-> info-> type ()-> toGeneric ();
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
		aff-> info-> value () = NULL;
		list.append (aff-> toGeneric ());
	    }
	}
	return list.getTree ();
    }
    
    Ymir::Tree IMatch::validateBlock (Expression test, Block block, Ymir::Tree elsePart, Ymir::Tree affectPart) {
	Ymir::Tree bool_expr = test-> toGeneric ();
	Ymir::Tree thenLabel = Ymir::makeLabel (this-> token.getLocus (), "then");
	Ymir::Tree endLabel = Ymir::makeLabel (this-> token.getLocus (), "end_if");
	
	Ymir::Tree goto_then = Ymir::buildTree (GOTO_EXPR, test-> token.getLocus (), void_type_node, thenLabel);
	Ymir::Tree goto_end = Ymir::buildTree (GOTO_EXPR, test-> token.getLocus (), void_type_node, endLabel);
	Ymir::Tree goto_else, elseLabel;
	
	if (elsePart.isNull ()) {
	    goto_else = goto_end;
	} else {
	    elseLabel = Ymir::makeLabel (this-> token.getLocus (), "else");
	    goto_else = Ymir::buildTree (GOTO_EXPR, test-> token.getLocus (), void_type_node, elseLabel);
	}

	Ymir::TreeStmtList list;
	Ymir::Tree cond_expr = Ymir::buildTree (COND_EXPR, test-> token.getLocus (), void_type_node, bool_expr, goto_then, goto_else);
	list.append (cond_expr);

	Ymir::Tree then_label_expr = Ymir::buildTree (LABEL_EXPR, block-> token.getLocus (), void_type_node, thenLabel);
	list.append (then_label_expr);
	Ymir::Tree then_part = block-> toGeneric ();
	list.append (affectPart);
	list.append (then_part);
	list.append (goto_end);
	
	if (!elseLabel.isNull ()) {
	    Ymir::Tree else_label_expr = Ymir::buildTree (LABEL_EXPR, test-> token.getLocus (), void_type_node, elseLabel);
	    list.append (else_label_expr);
	    list.append (elsePart);
	    list.append (goto_end);
	}
	
	Ymir::Tree endif_label_expr = Ymir::buildTree (LABEL_EXPR, this-> token.getLocus (), void_type_node, endLabel);
	list.append (endif_label_expr);

	return list.getTree ();	
    }

    Ymir::Tree IMatch::validateBlockExpr (Expression test, Block block, InfoType caster, Ymir::Tree res, Ymir::Tree elsePart, Ymir::Tree affectPart) {
	Ymir::Tree bool_expr = test-> toGeneric ();
	Ymir::Tree thenLabel = Ymir::makeLabel (this-> token.getLocus (), "then");
	Ymir::Tree endLabel = Ymir::makeLabel (this-> token.getLocus (), "end_if");
	Ymir::Tree goto_then = Ymir::buildTree (GOTO_EXPR, test-> token.getLocus (), void_type_node, thenLabel);
	Ymir::Tree goto_end = Ymir::buildTree (GOTO_EXPR, test-> token.getLocus (), void_type_node, endLabel);
	Ymir::Tree goto_else, elseLabel;
	
	if (elsePart.isNull ()) {
	    goto_else = goto_end;
	} else {
	    elseLabel = Ymir::makeLabel (this-> token.getLocus (), "else");
	    goto_else = Ymir::buildTree (GOTO_EXPR, test-> token.getLocus (), void_type_node, elseLabel);
	}

	Ymir::TreeStmtList list;
	Ymir::Tree cond_expr = Ymir::buildTree (COND_EXPR, test-> token.getLocus (), void_type_node, bool_expr, goto_then, goto_else);
	list.append (cond_expr);

	Ymir::Tree then_label_expr = Ymir::buildTree (LABEL_EXPR, block-> token.getLocus (), void_type_node, thenLabel);
	list.append (then_label_expr);
	Ymir::Tree resTree;
	InfoType type;
	Ymir::Tree then_part = block-> toGenericExpr (type, resTree);
		
	list.append (affectPart);
	list.append (then_part);
	list.append (Ymir::buildTree (MODIFY_EXPR, this-> token.getLocus (),
				      void_type_node,
				      res,
				      caster-> buildCastOp (this-> token,
							    caster, 
							    new (Z0) ITreeExpression (this-> token, type, resTree),
							    new (Z0) ITreeExpression (this-> token, caster, Ymir::Tree ()))
	));
	
	list.append (goto_end);							    
	
	if (!elseLabel.isNull ()) {
	    Ymir::Tree else_label_expr = Ymir::buildTree (LABEL_EXPR, test-> token.getLocus (), void_type_node, elseLabel);
	    list.append (else_label_expr);
	    list.append (elsePart);
	    list.append (goto_end);
	}

	Ymir::Tree endif_label_expr = Ymir::buildTree (LABEL_EXPR, this-> token.getLocus (), void_type_node, endLabel);
	list.append (endif_label_expr);
	
	return list.getTree ();	
    }
    
    Ymir::Tree IMatch::declareAndAffectAux () {
	Ymir::TreeStmtList list;
	auto decl = Ymir::makeAuxVar (this-> _aux-> token.getLocus (), ISymbol::getLastTmp (), this-> _aux-> info-> type ()-> toGeneric ());
	    
	this-> _aux-> info-> treeDecl (decl.getOperand (1));
	Ymir::getStackStmtList ().back ().append (decl.getOperand (0));
	list.append (this-> _binAux-> toGeneric ());
	
	return list.getTree ();
    }


    Ymir::Tree IMatch::toGenericExpression (Ymir::TreeStmtList list, Ymir::Tree elsePart) {
	auto aux = Ymir::makeAuxVar (this-> token.getLocus (), ISymbol::getLastTmp (), this-> info-> type ()-> toGeneric ());
	list.append (declareAndAffectAux ());	
	
	for (auto it : Ymir::r (this-> _soluce.size (), 0)) {
	    Ymir::Tree affectPart = declareVars (
		this-> _soluce [it - 1].created,
		this-> _soluce [it - 1].caster
	    );	    
	    
	    elsePart = validateBlockExpr (
		this-> _soluce [it - 1].test,
		this-> _block [it - 1],
		this-> _casters [it - 1],
		aux,
		elsePart,
		affectPart	    
	    );	    
	}
	
	list.append (elsePart);

	return Ymir::compoundExpr (this-> token.getLocus (), list.getTree (), aux);
    }
    
    Ymir::Tree IMatch::toGeneric () {
	Ymir::Tree elsePart;
	Ymir::TreeStmtList list;

	if (this-> info != NULL) {
	    return toGenericExpression (list, elsePart);
	}
	
	list.append (declareAndAffectAux ());	
	for (auto it : Ymir::r (this-> _soluce.size (), 0)) {
	    Ymir::Tree affectPart = declareVars (
		this-> _soluce [it - 1].created,
		this-> _soluce [it - 1].caster
	    );	    
	    
	    elsePart = validateBlock (
		this-> _soluce [it - 1].test,
		this-> _block [it - 1],
		elsePart,
		affectPart	    
	    );	    
	}
	
	list.append (elsePart);

	
	return list.getTree ();
    }

    Ymir::Tree IAssert::callPrint (Ymir::Tree msgTree) {
	auto fn = InternalFunction::getYError ().getTree ();
	
	std::string pre;	
	Ymir::TreeStmtList list;
		
	if (!msgTree.isNull ()) {
	    auto aux = Ymir::makeAuxVar (this-> token.getLocus (), ISymbol::getLastTmp (), msgTree.getType ());
	    list.append (buildTree (
		MODIFY_EXPR, this-> token.getLocus (), void_type_node, aux, msgTree
	    ));

	    auto treeExpr = new (Z0) ITreeExpression (this-> token, this-> _msg-> info-> type (), aux);
	    
	    tree nextArgs [] = {
		ArrayUtils::InstLen (this-> token, NULL, treeExpr, NULL).getTree (),
		ArrayUtils::InstPtr (this-> token, NULL, treeExpr, NULL).getTree ()
	    };
	    
	    list.append (
		build_call_array_loc (this-> token.getLocus (),
				      void_type_node, fn,
				      2,
				      nextArgs
		)		
	    );
	}

	auto abort_fn = InternalFunction::getAbort ();
	
	list.append (
	    build_call_array_loc (this-> token.getLocus (), void_type_node, abort_fn.getTree (), 0, NULL)
	);
	
	return list.getTree ();
    }

    Ymir::Tree IPragma::toGeneric () {
	if (this-> info-> isImmutable ()) {
	    auto ret = this-> info-> value ()-> toYmir (this-> info)-> toGeneric ();
	    return ret;
	}
	return Ymir::Tree ();
    }
    
    Ymir::Tree IAssert::toGeneric () {
	if (this-> _isStatic) return Ymir::Tree ();
	Ymir::Tree text;
	std::string format;
	if (this-> _msg) {
	    text = ArrayUtils::InstToArray (
		this-> token,
		this-> _msg-> info-> type (),
		this-> _msg,
		new (Z0) ITreeExpression (this-> token, this-> _msg-> info-> type (), Ymir::Tree ())
	    );
	}

	Ymir::Tree bool_expr = this-> _expr-> toGeneric ();
	Ymir::Tree thenLabel = Ymir::makeLabel (this-> token.getLocus (), "then");
	Ymir::Tree endLabel = Ymir::makeLabel (this-> token.getLocus (), "end");
	Ymir::Tree goto_then = Ymir::buildTree (GOTO_EXPR, this-> _expr-> token.getLocus (), void_type_node, thenLabel);
	Ymir::Tree goto_end = Ymir::buildTree (GOTO_EXPR, this-> _expr-> token.getLocus (), void_type_node, endLabel);

	Ymir::TreeStmtList list;
	Ymir::Tree cond_expr = Ymir::buildTree (COND_EXPR, this-> _expr-> token.getLocus (), void_type_node, bool_expr, goto_end, goto_then);
	list.append (cond_expr);
	Ymir::Tree then_label_expr = Ymir::buildTree (LABEL_EXPR, this-> _expr-> token.getLocus (), void_type_node, thenLabel);
	Ymir::Tree end_label_expr = Ymir::buildTree (LABEL_EXPR, this-> _expr-> token.getLocus (), void_type_node, endLabel);
	list.append (then_label_expr);
	list.append (callPrint (text));
	list.append (goto_end);
	list.append (end_label_expr);
	return list.getTree ();	
    }
    

    // Ymir::Tree IMacroCall::toGeneric () {
    // 	auto insts = this-> bl-> toGeneric ();
    // 	if (this-> expr) {
    // 	    auto expr = this-> expr-> toGeneric ();
    // 	    return Ymir::compoundExpr (this-> token.getLocus (), insts, expr);
    // 	}
    // 	return insts;
    // }

    Ymir::Tree IMacroRepeat::toGeneric () {
	return Ymir::Tree ();
    }

    Ymir::Tree IAffectGeneric::toGeneric () {
	auto rtree = this-> _right-> toGeneric ();
	return Ymir::buildTree (
	    MODIFY_EXPR,
	    this-> token.getLocus (),
	    void_type_node,
	    this-> _left-> toGeneric (),
	    this-> _addr ? Ymir::getAddr (rtree) : rtree
	);
    }
    
}




