#include <ymir/semantic/pack/FinalFrame.hh>
#include <ymir/ast/Var.hh>
#include <ymir/ast/Block.hh>
#include <ymir/ast/Expression.hh>
#include <ymir/semantic/tree/Tree.hh>
#include <ymir/semantic/types/InfoType.hh>
#include <ymir/semantic/types/VoidInfo.hh>
#include <ymir/semantic/types/TupleInfo.hh>
#include <ymir/semantic/types/RefInfo.hh>
#include <ymir/utils/Mangler.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/semantic/types/CharInfo.hh>
#include <ymir/syntax/Keys.hh>

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "target.h"
#include "tree.h"
#include "tree-iterator.h"
#include "input.h"
#include "diagnostic.h"
#include "stringpool.h"
#include "cgraph.h"
#include "gimplify.h"
#include "gimple-expr.h"
#include "convert.h"
#include "print-tree.h"
#include "stor-layout.h"
#include "fold-const.h"
#include "print-tree.h"

namespace semantic {

    Ymir::Tree IFinalFrame::__fn_decl__;
    Ymir::Tree IFinalFrame::__fn_closure__;
    std::map <std::string, Ymir::Tree> IFinalFrame::__declared__;
    std::vector <Ymir::Tree> IFinalFrame::__contextToAdd__;
    std::vector <Ymir::Tree> IFinalFrame::__isInlining__;
    std::vector <Ymir::Tree> IFinalFrame::__endLabel__;
    std::vector <FinalFrame> IFinalFrame::__inlining__;
    
    IFinalFrame::IFinalFrame (Symbol type, Namespace space, std::string name, const std::vector<syntax::Var> & vars, syntax::Block bl, const std::vector <syntax::Expression>& tmps) :
	_type (type),
	_file (""),
	_space (space),
	_name (name),
	_vars (vars),
	_tmps (tmps),
	_isVariadic (false),
	_isInline (false),
	_block (bl)
    {}
    
    std::string &IFinalFrame::name () {
	return this-> _name;
    }

    Namespace IFinalFrame::space () {
	return this-> _space;
    }

    std::string& IFinalFrame::file () {
	return this-> _file;
    }

    Symbol IFinalFrame::type () {
	return this-> _type;
    }

    bool& IFinalFrame::isVariadic () {
	return this-> _isVariadic;
    }

    bool& IFinalFrame::isInline () {
	return this-> _isInline;
    }

    bool& IFinalFrame::isMoved () {
	return this-> _isMoved;
    }
    
    std::vector<syntax::Var>& IFinalFrame::vars () {
	return this-> _vars;
    }

    std::vector<syntax::Var>& IFinalFrame::closure () {
	return this-> _closure;
    }

    Ymir::Tree IFinalFrame::getCurrentClosure () {
	return __fn_closure__;
    }
    
    std::vector <syntax::Expression>& IFinalFrame::tmps () {
	return this-> _tmps;
    }

    void IFinalFrame::isForcedDelegate () {
	this-> _isForced = true;
    }

    syntax::Block IFinalFrame::block () {
    	return this-> _block;
    }
    
    Ymir::Tree& IFinalFrame::currentFrame () {
	return __fn_decl__;
    }

    Ymir::Tree IFinalFrame::getDeclaredType (const char* name) {
       	auto type = __declared__.find (name);
	if (type == __declared__.end ()) return Ymir::Tree ();
	return type-> second;
    }
    
    void IFinalFrame::declareType (std::string &name, Ymir::Tree type) {
	if (__declared__.find (name) == __declared__.end ()) {
	    Ymir::Tree decl = build_decl (BUILTINS_LOCATION, TYPE_DECL,
					  get_identifier (name.c_str ()),
					  type.getTree ()
	    );

	    if (__fn_decl__.isNull ()) {
		__contextToAdd__.push_back (decl);
	    } else {
		DECL_CONTEXT (decl.getTree ()) = __fn_decl__.getTree ();
		Ymir::getStackVarDeclChain ().back ().append (decl);
	    }
	    
	    __declared__ [name] = type;
	}	    
    }

    void IFinalFrame::declareType (const char* name, Ymir::Tree type) {
	if (__declared__.find (name) == __declared__.end ()) {
	    Ymir::Tree decl = build_decl (BUILTINS_LOCATION, TYPE_DECL,
					  get_identifier (name),
					  type.getTree ()
	    );


	    if (__fn_decl__.isNull ()) {
		__contextToAdd__.push_back (decl);
	    } else {
		DECL_CONTEXT (decl.getTree ()) = __fn_decl__.getTree ();
		Ymir::getStackVarDeclChain ().back ().append (decl);
	    }
	   	    
	    __declared__ [name] = type;
	}	    
    }

    Ymir::Tree IFinalFrame::createClosureType () {
	auto name = Namespace (this-> space (), this-> _name).toString () + ".closure";
	if (this-> _closure.size () != 0) {
	    std::vector <InfoType> types;
	    std::vector <std::string> attrs;
	    for (auto it : this-> _closure) {
		if (!this-> _isMoved)
		    types.push_back (new (Z0) IRefInfo (false, it-> info-> type));
		else
		    types.push_back (it-> info-> type);
		attrs.push_back (it-> info-> sym.getStr ());
	    }
	    
	    return build_pointer_type (Ymir::makeTuple (name, types, attrs).getTree ());
	} else if (this-> _isForced) {
	    return build_pointer_type (Ymir::makeTuple (name, {new (Z0) ICharInfo (false)}).getTree ());
	}
	
	return Ymir::Tree ();
    }
    
    void IFinalFrame::declArguments (Ymir::Tree closureType) {
	Ymir::Tree arglist;
	if (!closureType.isNull ()) {
	    __fn_closure__ = build_decl (
		BUILTINS_LOCATION,
		PARM_DECL,
		get_identifier (Keys::SELF.c_str ()),
		closureType.getTree ()
	    );
	    
	    DECL_CONTEXT (__fn_closure__.getTree ()) = __fn_decl__.getTree ();
	    DECL_ARG_TYPE (__fn_closure__.getTree ()) = TREE_TYPE (__fn_closure__.getTree ());
	    arglist = chainon (arglist.getTree (), __fn_closure__.getTree ());
	    TREE_USED (__fn_closure__.getTree ()) = 1;
	}
	
	for (auto var : this-> _vars) {
	    Ymir::Tree decl = build_decl (
		var-> token.getLocus (),
		PARM_DECL,
		get_identifier (var-> token.getStr ().c_str ()),
		var-> info-> type-> toGeneric ().getTree ()
	    );
	    
	    DECL_CONTEXT (decl.getTree ()) = __fn_decl__.getTree ();	    
	    DECL_ARG_TYPE (decl.getTree ()) = TREE_TYPE (decl.getTree ());
	    
	    arglist = chainon (arglist.getTree (), decl.getTree ());
	    TREE_USED (decl.getTree ()) = 1;
	    var-> info-> treeDecl (decl);
	}
	DECL_ARGUMENTS (__fn_decl__.getTree ()) = arglist.getTree ();	
    }

    Ymir::Tree IFinalFrame::declInlineArgs (std::vector <tree> params) {
	Ymir::TreeStmtList list;
	auto i = 0;
	for (auto var : this-> _vars) {	    
	    Ymir::Tree decl = build_decl (
		var-> token.getLocus (),
		VAR_DECL,
		get_identifier (var-> token.getStr ().c_str ()),
		var-> info-> type-> toGeneric ().getTree ()
	    );
	    
	    DECL_CONTEXT (decl.getTree ()) = __fn_decl__.getTree ();	    	    
	    var-> info-> treeDecl (decl);
	    Ymir::getStackVarDeclChain ().back ().append (decl);
	    list.append (Ymir::buildTree (DECL_EXPR, var-> token.getLocus (), void_type_node, decl));
	    list.append (Ymir::buildTree (MODIFY_EXPR, var-> token.getLocus (), void_type_node, decl, params [i]));
	    i++;
	}
	return list.getTree ();
    }
    
    Ymir::Tree IFinalFrame::callInline (Word where, std::vector <tree> params) {
	Ymir::TreeStmtList list;
	for (auto it : __inlining__) {
	    if (it == this) {
		Ymir::Error::recursiveInlining (where);
		return build_int_cst_type (long_unsigned_type_node, 0);	   
	    }
	}
	
	__inlining__.push_back (this);
	list.append (declInlineArgs (params));
	auto endLabel = Ymir::makeLabel (BUILTINS_LOCATION, "end");
	if (!this-> _type-> type-> is <IVoidInfo> ()) {
	    tree ret = this-> _type-> type-> toGeneric ().getTree ();
	    auto var = Ymir::makeAuxVar (BUILTINS_LOCATION, ISymbol::getLastTmp (), ret);
	    IFinalFrame::__isInlining__.push_back (var);
	    IFinalFrame::__endLabel__.push_back (endLabel);
	    auto inside = this-> _block-> toGeneric ();
	    list.append (inside.getTree ());
	    IFinalFrame::__isInlining__.pop_back ();
	    IFinalFrame::__endLabel__.pop_back ();
	    list.append (Ymir::buildTree (LABEL_EXPR, this-> _block-> token.getLocus (), void_type_node, endLabel));
	    __inlining__.pop_back ();
	    
	    return Ymir::compoundExpr (BUILTINS_LOCATION, list.getTree (), var);
	} else {
	    IFinalFrame::__isInlining__.push_back (Ymir::Tree ());
	    IFinalFrame::__endLabel__.push_back (endLabel);
	    auto inside = this-> _block-> toGeneric ();
	    list.append (inside.getTree ());
	    IFinalFrame::__isInlining__.pop_back ();
	    IFinalFrame::__endLabel__.pop_back ();
	    list.append (Ymir::buildTree (LABEL_EXPR, this-> _block-> token.getLocus (), void_type_node, endLabel));
	    __inlining__.pop_back ();
	    return list.getTree ();
	}
    }

    Ymir::Tree IFinalFrame::isInlining () {       
	if (IFinalFrame::__isInlining__.size () != 0)
	    return IFinalFrame::__isInlining__.back ();
	return Ymir::Tree ();
    }
       
    Ymir::Tree IFinalFrame::endLabel () {       
	if (IFinalFrame::__endLabel__.size () != 0)
	    return IFinalFrame::__endLabel__.back ();
	return Ymir::Tree ();
    }
    
    void IFinalFrame::finalize () {
	if (this-> isInline ()) return;
	
	ISymbol::resetNbTmp ();
	__declared__.clear ();
	__contextToAdd__.clear ();
	
	__inlining__.push_back (this);
	auto lastErrors = Ymir::Error::nb_errors;
	
	std::vector <tree> args (this-> _vars.size ());
	for (uint i = 0 ; i < this-> _vars.size () ; i++)
	    args [i] = this-> _vars [i]-> info-> type-> toGeneric ().getTree ();

	Ymir::Tree closureType = createClosureType ();
	if (!closureType.isNull ()) {
	    args.insert (args.begin (), closureType.getTree ());	    
	} 
	
	tree ret;
	if (this-> _name == Keys::MAIN && this-> _type-> type-> is<IVoidInfo> ())
	    ret = int_type_node;
	else
	    ret = this-> _type-> type-> toGeneric ().getTree ();
		
	//tree ident = get_identifier (this-> _name.c_str ());
	tree ident = get_identifier (Namespace (this-> space (), this-> _name).toString ().c_str ());
	tree ident_ASM = get_identifier (Mangler::mangle_function (this-> _name, this).c_str ());
	
	tree fntype = build_function_type_array (ret, args.size (), args.data ());
	tree fn_decl = build_decl (BUILTINS_LOCATION, FUNCTION_DECL, ident, fntype);	
	SET_DECL_ASSEMBLER_NAME (fn_decl, ident_ASM);// = asmIdent;
	
	Ymir::currentContext () = fn_decl;
	__fn_decl__ = fn_decl;
	
	this-> declArguments (closureType);

	Ymir::enterBlock ();
	tree result_decl = build_decl (BUILTINS_LOCATION, RESULT_DECL,
				       NULL_TREE, ret);
	
	DECL_RESULT (fn_decl) = result_decl;
	
	Ymir::Tree inside = this-> _block-> toGeneric ();	
	Ymir::getStackStmtList ().back ().append (inside);	

	if (this-> _name == Keys::MAIN && this-> _type-> type-> is <IVoidInfo> ()) {
	    Ymir::Tree inside = Ymir::buildTree (
		MODIFY_EXPR, BUILTINS_LOCATION, void_type_node, result_decl, build_int_cst_type (int_type_node, 0)
	    );

	    Ymir::getStackStmtList ().back ().append (Ymir::buildTree (
		RETURN_EXPR, BUILTINS_LOCATION, void_type_node, inside
	    ));
	}	    
	auto fnTreeBlock = Ymir::leaveBlock ();
	auto fnBlock = fnTreeBlock.block;
	__inlining__.pop_back ();
	
	BLOCK_SUPERCONTEXT (fnBlock.getTree ()) = fn_decl;
	DECL_INITIAL (fn_decl) = fnBlock.getTree ();
	DECL_SAVED_TREE (fn_decl) = fnTreeBlock.bind_expr.getTree ();

	DECL_EXTERNAL (fn_decl) = 0;
	DECL_PRESERVE_P (fn_decl) = 1;

	TREE_PUBLIC (fn_decl) = 1;
	TREE_STATIC (fn_decl) = 1;

	if (Ymir::Error::nb_errors > lastErrors)
	    return;
	
	gimplify_function_tree (fn_decl);
	cgraph_node::finalize_function (fn_decl, true);
	fn_decl = NULL_TREE;
	__fn_decl__ = NULL_TREE;
    }
    
    
}
