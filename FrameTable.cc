#include "semantic/pack/FrameTable.hh"
#include <ymir/semantic/pack/Frame.hh>
#include <ymir/semantic/pack/FinalFrame.hh>
#include <ymir/semantic/pack/InternalFunction.hh>
#include <ymir/semantic/tree/Tree.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/ast/Global.hh>
#include <ymir/utils/Mangler.hh>
#include <ymir/semantic/object/AggregateInfo.hh>

namespace semantic {
    
    FrameTable FrameTable::__instance__;
    
    void FrameTable::insert (Frame fr) {
	this-> _pures.push_back (fr);
    }

    void FrameTable::insert (StructCstInfo info) {
	this-> _structs.push_back (info);
    }

    void FrameTable::insert (AggregateCstInfo info) {
	this-> _aggrs.push_back (info);
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

    void FrameTable::insert (syntax::Global sym) {
	this-> _globals.push_back (sym);
    }

    void FrameTable::insertExtern (Symbol sym) {
	this-> _externals.push_back (sym);
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

    FrameProto FrameTable::getProto (FrameProto& proto) {
	for (auto it : this-> _protos) {
	    if (it-> equals (proto)) {
		proto-> type () = it-> type ();
		return it;
	    }
	}
	return NULL;
    }

    bool FrameTable::existsFinal (FrameProto & proto) {
	for (auto it : this-> _protos) {
	    if (it-> equals (proto) && it-> attached () != NULL) {
		proto-> type () = it-> type ();
		return true;
	    }
	}
	return false;
    }
    
    
    void FrameTable::identify (StructInfo) {
	// auto name = info-> onlyNameTypeString ();
	// auto val = this-> _structIds.find (name);
	// if (val == this-> _structIds.end ()) {
	//     auto id = this-> _currentStructId;
	//     this-> _currentStructId ++;
	//     info-> setId (id);
	//     this-> _structIds [name] = {info};
	// } else {	    
	//     for (auto it : val-> second) {
	// 	if (info-> getTypes ().size () == it-> getTypes ().size ()) {
	// 	    bool same = true;
	// 	    for (auto it_ : Ymir::r (0, info-> getTypes ().size ())) {
	// 		if (!info-> getTypes () [it_]-> isSame (it-> getTypes () [it_])) {
	// 		    same = false;
	// 		    break;
	// 		}
	// 	    }

	// 	    if (same) {
	// 		info-> setId (it-> getStrId ());
	// 		return;
	// 	    }
	// 	}
	//     }
	//     auto id = this-> _currentStructId;
	//     this-> _currentStructId ++;
	//     info-> setId (id);
	//     this-> _structIds [name].push_back (info);
	// }
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

    std::vector <syntax::Global> & FrameTable::globals () {
	return this-> _globals;
    }
   
    std::vector <Symbol> & FrameTable::externals () {
	return this-> _externals;
    }

    bool FrameTable::isSuccessor (Namespace space, AggregateCstInfo info) {
	for (auto it : this-> _aggrs) {
	    auto _space = Namespace (it-> space (), it-> name ());
	    if (space == _space) {
		return it-> isSuccessor (info);
	    }
	}
	return false;
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
	auto aux = Ymir::makeAuxVar (BUILTINS_LOCATION, ISymbol::getLastTmp (), ret);
	
	Ymir::Tree inside = Ymir::buildTree (
	    MODIFY_EXPR, BUILTINS_LOCATION, void_type_node, aux,
	    build_call_array_loc (
		BUILTINS_LOCATION,
		void_type_node,
		fn_run_main,
		vars.size (),
		vars.data ()
	    )
	);
	
	Ymir::getStackStmtList ().back ().append (allSelfCall ());
	Ymir::getStackStmtList ().back ().append (inside);
	Ymir::getStackStmtList ().back ().append (allSelfDestCall ());
	    
	Ymir::Tree result_expr = Ymir::buildTree (
	    MODIFY_EXPR, BUILTINS_LOCATION, void_type_node, result_decl, aux
	);
	
	Ymir::getStackStmtList ().back ().append (Ymir::buildTree (
	    RETURN_EXPR, BUILTINS_LOCATION, void_type_node, result_expr
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
    
    Ymir::Tree FrameTable::allSelfDestCall () {
	Ymir::TreeStmtList list;
	auto modules = Table::instance ().modules ();
	for (auto it : modules) {
	    auto fr = Table::instance ().moduleDestruct (it);
	    if (fr != NULL) {
		auto proto = fr-> validate ();
		list.append (
		    build_call_array_loc (
			BUILTINS_LOCATION,
			void_type_node, 
			proto-> toGeneric ().getTree (),
			0, NULL
		    )
		);
	    }
	}
	return list.getTree (); 
    }

    Ymir::Tree FrameTable::allSelfCall () {
	Ymir::TreeStmtList list;
	auto modules = Table::instance ().modules ();
	for (auto it : modules) {
	    auto fr = Table::instance ().moduleConstruct (it);
	    if (fr != NULL) {
		auto proto = fr-> validate ();
		list.append (
		    build_call_array_loc (
			BUILTINS_LOCATION,
			void_type_node, 
			proto-> toGeneric ().getTree (),
			0, NULL
		    )
		);
	    }
	}
	return list.getTree ();
    }

       
    void FrameTable::purge () {
	this-> _finals.clear ();
	this-> _finalTemplates.clear ();
	this-> _pures.clear ();
	this-> _protos.clear ();
    }
    


}
