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

	    aux-> sym = SymbolPtr (new Symbol (aux-> token, new UndefInfo ()));
	    Table::instance ().insert (aux-> token-> getStr (), aux-> sym);
	}

	for (auto aff : this-> affects) {
	    toAffects.push_back (aff-> expression ());
	}	
		
	return new VarDecl (this-> token, toDecl, toAffects);	    
    }

    void VarDecl::print (int nb) {
	printf ("%*c<VarDecl> ", nb, ' ');
	for (auto var : this-> decls) {
	    var-> print (nb + 4);
	}

	for (auto aff : this-> affects) {
	    aff-> print (nb + 4);
	}
    }


};
