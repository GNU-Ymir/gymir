#include "VarDecl.hh"
#include "Error.hh"
#include "Table.hh"
#include "UndefInfo.hh"

namespace Syntax {

    using namespace Semantic;

    ExpressionPtr VarDecl::expression () {
	if (this-> var-> isType ()) {
	    Ymir::Error::append (var-> token-> getLocus (), 
				 "Definition d'une variable qui est un type '%s'",
				 var-> token-> getCstr ());
	    return NULL;
	} else if (this-> var-> templates.size () > 0) {
	    Ymir::Error::append (var-> token-> getLocus (),
				 "Template sur la definition d'une variable impossible '%s'",
				 var-> token-> getCstr ());
	    return NULL;
	}
	auto aux = new VarDecl (this);
	auto info = Table::instance ().get (aux-> var-> token-> getStr ());
	if (!info.isVoid ()) {
	    Ymir::Error::append (var-> token-> getLocus (),
				 "Redefinition de la variable '%s'",
				 var-> token-> getCstr ());

	    Ymir::Error::note (info.token-> getLocus (),
			       "Definis la premiere fois ici");
	    return NULL;
	} 
	aux-> info = new UndefInfo ();
	Table::instance ().insert (aux-> var-> token-> getStr (), Symbol (aux-> var-> token, aux-> info));
	aux-> info = info.type;
	aux-> var-> info = info.type;
	return aux;	    
    }

    void VarDecl::print (int nb) {
	printf ("%*c<VarDecl> ", nb, ' ');
	var-> print (nb + 4);
    }


};
