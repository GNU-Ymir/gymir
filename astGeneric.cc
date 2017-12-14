#include <ymir/ast/_.hh>
#include <ymir/semantic/tree/Tree.hh>
#include <ymir/semantic/tree/Generic.hh>
#include <ymir/semantic/types/_.hh>
#include <ymir/errors/Error.hh>
#include <ymir/semantic/pack/FinalFrame.hh>

namespace syntax {
    using namespace semantic;
    
    Ymir::Tree IInstruction::toGeneric () {
	Ymir::Error::assert ((std::string ("TODO ") + this-> getId ()).c_str  ());
    }
    
    Ymir::Tree IBlock::toGenericNoEntry () {
	for (auto it : this-> insts) {
	    Ymir::getStackStmtList ().back ().append (it-> toGeneric ());
	}
	return Ymir::Tree ();
    }
    
    Ymir::Tree IBlock::toGeneric () {
	Ymir::enterBlock ();
	for (auto it : this-> insts) {
	    Ymir::getStackStmtList ().back ().append (it-> toGeneric ());
	}
	return Ymir::leaveBlock ().block;
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
    
}




