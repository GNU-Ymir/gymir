#include "Var.hh"
#include "Table.hh"
#include "Error.hh"
#include "UndefInfo.hh"
#include <iostream>
#include "tree-pretty-print.h"

namespace Syntax {
    
    using namespace Semantic;   
    
    void Var::print (int nb) {
	printf("%*c<Var> ", nb, ' ');
	token -> print ();
	if (templates.size () > 0) {
	    printf ("!(");
	    for (int i = 0; i < (int)templates.size (); i++) {
		if (templates[i] != NULL)
		    templates[i]->print (-1);
		if (i < (int)templates.size () - 1) printf(", ");
	    }
	    printf (")");
	}
	if (nb != -1)
	    printf ("\n");
    }

    VarPtr Var::declare (const char*) {
	fatal_error (this->token->getLocus (),
		  "Erreur interne");
	return this;
    }
    
    VarPtr Var::asType () {
	auto info = Table::instance ().get (this->token->getStr ());
	if (!info-> isVoid() && !info-> type->Is(STRUCT)) {
	    Ymir::Error::append (this->token->getLocus (),
		      "%s n'est pas un type",
		      this->token->getCstr());
	    return this;
	} else if (info-> isVoid ()) {
	    std::vector<ExpressionPtr> tpls;
	    for (int i = 0; i < (int)this->templates.size (); i++) {
		tpls.push_back (this->templates [i] -> expression ());
	    }
	    auto t_info = TypeInfo::create_type (this->token, tpls);
	    if (t_info == NULL) {
		Ymir::Error::append (this->token->getLocus (),
			  "Type inconnu %s",
			  this->token->getCstr());
		return NULL;
	    }
	    return new Type (this->token, Semantic::SymbolPtr (new Semantic::Symbol (this-> token, t_info)));
	} else {	    
	    //TODO, structure
	    Ymir::Error::fatal (this-> token-> getLocus (),
				"Erreur interne, non iplemente %s.asType ()",
				AstEnums::toString (this-> type));
	    return NULL;
	}
    }

    VarPtr Var::expression () {
	if (!this-> isType () && this->templates.size () == 0) {
	    auto aux = new Var (this->token);
	    auto info = Table::instance ().get (this->token->getStr());
	    if (info-> isVoid ()) {
		Ymir::Error::append (this-> token-> getLocus (),
				     "Variable inconnu '%s'",
				     this-> token-> getCstr ());
		return NULL;
	    }

	    aux-> sym = info;
	    return aux;
	} else return this-> asType ();
    }

    bool Var::isType () {
	auto info = Table::instance ().get (this->token->getStr ());
	if (info-> isVoid ()) {
	    return TypeInfo::exist_type (this->token);
	} else return info-> type->Is(STRUCT);
    }
    
    VarPtr TypedVar::declare (const char * funName) {
	auto it = Table::instance().get (this->token->getStr ());
	if (it-> isVoid()) {
	    auto type = this->type->asType ();
	    if (type == NULL) return NULL;
	    else this-> sym = Semantic::SymbolPtr (new Symbol (this-> token, type-> getType ()-> type));
	    
	    Table::instance ().insert (this->token->getStr (), this-> sym);
	    return this;
	} else {
	    Ymir::Error::append (this->token->getLocus(),
		      "Redefinition du parametre %s.%s",
		      funName, 
		      this->token->getStr().c_str());		      
	}
    }
    
    Ymir::Tree Var::treeExpr () {
	return this-> sym-> treeDecl ();
    }

    void TypedVar::print (int nb) {
	printf("%*c<TypedVar> ", nb, ' ');
	token -> print ();
	if (type != NULL) type->print ();
    }
    

};
