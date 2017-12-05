#include <ymir/ast/_.hh>
#include <ymir/semantic/tree/Tree.hh>
#include <ymir/errors/Error.hh>

namespace syntax {

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

	    var-> info-> treeDecl (decl);
	    Ymir::getStackStmtList ().back ().append (decl);
	    list.append (buildTree (DECL_EXPR, var-> token.getLocus (), void_type_node, decl));

	    if (aff != NULL) {
		list.append (aff-> toGeneric ());
	    }
	}
	return list.getTree ();
    }

    Ymir::Tree IFixed::toGeneric () {
	switch (this-> type) {
	case FixedConst::BYTE : return build_int_cst_type (signed_char_type_node, atoi (this-> token.getStr ().c_str ()));
	case FixedConst::UBYTE : return build_int_cst_type (unsigned_char_type_node, atoi (this-> token.getStr ().c_str ()));
	case FixedConst::SHORT : return build_int_cst_type (short_integer_type_node, atoi (this-> token.getStr ().c_str ()));
	case FixedConst::USHORT : return build_int_cst_type (short_unsigned_type_node, atoi (this-> token.getStr ().c_str ()));
	case FixedConst::INT : return build_int_cst_type (integer_type_node, atoi (this-> token.getStr ().c_str ()));
	case FixedConst::UINT : return build_int_cst_type (unsigned_type_node, atoi (this-> token.getStr ().c_str ()));
	case FixedConst::LONG : return build_int_cst_type (long_integer_type_node, atoi (this-> token.getStr ().c_str ()));
	case FixedConst::ULONG : return build_int_cst_type (long_unsigned_type_node, atoi (this-> token.getStr ().c_str ()));
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
    
    
}




