#include "VarDecl.hh"
#include "Error.hh"
#include "Table.hh"
#include "UndefInfo.hh"

namespace Syntax {

    using namespace Semantic;

    InstructionPtr VarDecl::instruction () {
	std::vector<VarPtr> toDecl;
	std::vector<ExpressionPtr> toAffects;
	for (auto var : this-> decls) {
	    if (var-> isType ()) {
		Ymir::Error::append (var-> token-> getLocus (), 
				     "Definition d'une variable qui est un type '%s'",
				     var-> token-> getCstr ());
		return NULL;
	    } else if (var-> templates.size () > 0) {
		Ymir::Error::append (var-> token-> getLocus (),
				     "Template sur la definition d'une variable impossible '%s'",
				     var-> token-> getCstr ());
		return NULL;
	    }

	    auto aux = new Var (var-> token);
	    auto info = Table::instance ().get (var-> token-> getStr ());	    
	    if (!info-> isVoid ()) {
		Ymir::Error::append (var-> token-> getLocus (),
				     "Redefinition de la variable '%s'",
				     var-> token-> getCstr ());
		
		Ymir::Error::note (info-> token-> getLocus (),
				   "Definis la premiere fois ici");
		return NULL;
	    } 

	    aux-> sym = new Symbol (aux-> token, new UndefInfo ());
	    Table::instance ().insert (aux-> token-> getStr (), aux-> sym);
	    toDecl.push_back (aux);
	}

	for (auto aff : this-> affects) {
	    toAffects.push_back (aff-> expression ());
	}	
		
	return new VarDecl (this-> token, toDecl, toAffects);	    
    }


    Ymir::Tree VarDecl::statement () {
	Ymir::TreeStmtList list;
	for (int i = 0 ; i < (int) this->decls.size () ; i++) {
	    auto var = this->decls [i];
	    auto aff = this->affects [i];
	    auto type_tree = var-> getType ()->  type-> toGeneric ();
	    Ymir::Tree decl = build_decl (
		var-> token-> getLocus (),
		VAR_DECL,
		get_identifier (var-> token-> getStr ().c_str ()),
		type_tree.getTree ()
	    );

	    var-> sym-> treeDecl (decl);
	    
	    Ymir::getStackVarDeclChain ().back ().append (decl);
	    list.append (buildTree (DECL_EXPR, var-> token-> getLocus (), void_type_node, decl));

	    if (aff != NULL) {
	    	list.append (aff-> treeExpr ());
	    }
	}
	return list.getTree ();
    }
    
    void VarDecl::print (int nb) {
	printf ("%*c<VarDecl> \n", nb, ' ');

	for (auto var : this-> decls) {
	    var-> print (nb + 4);
	}

	for (auto aff : this-> affects) {
	    aff-> print (nb + 4);
	}
    }


};
