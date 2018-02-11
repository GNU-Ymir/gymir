#include "semantic/pack/FrameTable.hh"
#include <ymir/semantic/pack/Frame.hh>
#include <ymir/semantic/pack/FinalFrame.hh>
#include <ymir/semantic/pack/InternalFunction.hh>
#include <ymir/semantic/tree/Tree.hh>
#include <ymir/utils/Mangler.hh>

namespace semantic {
    
    FrameTable FrameTable::__instance__;
    
    void FrameTable::insert (Frame fr) {
	this-> _pures.push_back (fr);
    }

    void FrameTable::insert (StructCstInfo info) {
	this-> _structs.push_back (info);
    }
    
    void FrameTable::insert (FinalFrame fr) {
	this-> _finals.push_back (fr);
    }
	
    void FrameTable::insertTemplate (FinalFrame fr) {
	this-> _finalTemplates.push_back (fr);
    }

    void FrameTable::insert (FrameProto fr) {
	this-> _protos.push_back (fr);
    }

    void FrameTable::insert (Symbol sym) {
	this-> _globals.push_back (sym);
    }

    void FrameTable::insertExtern (Symbol sym) {
	this-> _globals.push_back (sym);
    }
    
    FinalFrame FrameTable::existsFinal (std::string name) {
	for (auto it : this-> _finals) {
	    if (it-> name () == name) return it;
	}

	for (auto it : this-> _finalTemplates) {
	    if (it-> name () == name) return it;
	}
	
	return NULL;
    }

    bool FrameTable::existsProto (FrameProto& proto) {
	for (auto it : this-> _protos) {
	    if (it-> equals (proto)) {
		proto-> type () = it-> type ();
		return true;
	    }
	}
	return false;
    }
    
    //StructCstInfo existsStruct (std::string);

    std::vector <Frame> & FrameTable::pures () {
	return this-> _pures;
    }

    std::vector <FinalFrame>& FrameTable::finals () {
	return this-> _finals;
    }

    std::vector <FinalFrame>& FrameTable::templates () {
	return this-> _finalTemplates;
    }

    std::vector <StructCstInfo> & FrameTable::structs () {
	return this-> _structs;
    }

    std::vector <Symbol> & FrameTable::globals () {
	return this-> _globals;
    }
   
    std::vector <Symbol> & FrameTable::externals () {
	return this-> _externals;
    }
    
    std::vector <tree> declArguments (Ymir::Tree func) {
	std::vector <tree> vars;
	Ymir::Tree arglist;	
	for (auto it : Ymir::r (0, 2)) {
	    tree decl;
	    if (it == 0) {
		decl = build_decl (
		    BUILTINS_LOCATION,
		    PARM_DECL,
		    get_identifier ("argc"),
		    int_type_node
		);
	    } else {
		decl = build_decl (
		    BUILTINS_LOCATION,
		    PARM_DECL,
		    get_identifier ("argv"),
		    build_pointer_type (build_pointer_type (ubyte_type_node))
		);
	    }

	    DECL_CONTEXT (decl) = func.getTree ();	    
	    DECL_ARG_TYPE (decl) = TREE_TYPE (decl);
	    
	    arglist = chainon (arglist.getTree (), decl);
	    TREE_USED (decl) = 1;
	    vars.push_back (decl);
	}
	DECL_ARGUMENTS (func.getTree ()) = arglist.getTree ();
	return vars;
    }
    
    void FrameTable::addMain () {
	tree ident = get_identifier ("main");
	tree args[] = {int_type_node, build_pointer_type (build_pointer_type (ubyte_type_node))};
	tree ret = int_type_node;

	tree fntype = build_function_type_array (ret, 2, args);
	tree fn_decl = build_decl (BUILTINS_LOCATION, FUNCTION_DECL, ident, fntype);
	
	Ymir::currentContext () = fn_decl;
	IFinalFrame::currentFrame () = fn_decl;
	auto vars = declArguments (fn_decl);
	vars.push_back (InternalFunction::getYMainPtr ().getTree ());

	tree fn_run_main = InternalFunction::getYRunMain ().getTree ();
	Ymir::enterBlock ();
	tree result_decl = build_decl (BUILTINS_LOCATION, RESULT_DECL, NULL_TREE, ret); 
	DECL_RESULT (fn_decl) = result_decl;

	Ymir::Tree inside = Ymir::buildTree (
	    MODIFY_EXPR, BUILTINS_LOCATION, void_type_node, result_decl,
	    build_call_array_loc (
		BUILTINS_LOCATION,
		void_type_node,
		fn_run_main,
		vars.size (),
		vars.data ()
	    )
	);
	
	Ymir::getStackStmtList ().back ().append (Ymir::buildTree (
	    RETURN_EXPR, BUILTINS_LOCATION, void_type_node, inside
	));

	auto fnTreeBlock = Ymir::leaveBlock ();
	auto fnBlock = fnTreeBlock.block;

	BLOCK_SUPERCONTEXT (fnBlock.getTree ()) = fn_decl;
	DECL_INITIAL (fn_decl) = fnBlock.getTree ();
	DECL_SAVED_TREE (fn_decl) = fnTreeBlock.bind_expr.getTree ();

	DECL_EXTERNAL (fn_decl) = 0;
	DECL_PRESERVE_P (fn_decl) = 1;

	TREE_PUBLIC (fn_decl) = 1;
	TREE_STATIC (fn_decl) = 1;
	
	gimplify_function_tree (fn_decl);
	cgraph_node::finalize_function (fn_decl, true);
	fn_decl = NULL_TREE;
	IFinalFrame::currentFrame () = NULL;
    }
    
    void FrameTable::purge () {
	this-> _finals.clear ();
	this-> _finalTemplates.clear ();
	this-> _pures.clear ();
	this-> _protos.clear ();
    }
    
}
