#include "Function.hh"
#include "Table.hh"
#include "FunctionInfo.hh"
#include "config.h"
#include "coretypes.h"
#include "input.h"
#include "diagnostic.h"
#include "PureFrames.hh"

namespace Syntax {
    
    using namespace Semantic;

    const char * Function::MAIN = "main";
    
    void Function::print (int nb) {
	printf ("%*c", nb, ' ');
	printf("<Function> ");
	token -> print ();	
	if (type != NULL)
	    type -> print (-1);
	printf ("\n");
	for (auto it : params)
	    if (it != NULL) it -> print (nb + 4);
	if (block != NULL)
	    block -> print (nb + 4);
    }

    void Function::declare () {
	if (this->token->getStr() == MAIN) {
	    PureFrames::insertPure (PureFrame (this, ""));
	} else {
	    this->space = Table::instance ().space ();
	    auto it = Table::instance().get (this->token->getStr ());
	    if (it-> isVoid ()) {
		FunctionInfo * fun = new FunctionInfo (this->token->getStr());
		fun->insert (Frame (this, space));
		Table::instance().insert (this->token->getStr(), SymbolPtr (new Symbol (this->token, fun)));
	    } else {
		if (it-> type-> Is(Semantic::FUNCTION)) {
		    auto fun = (FunctionInfo*)it-> type;
		    fun->insert (Frame (this, space));
		} else {
		    error_at (this-> token-> getLocus (),
			      "Fonction %s, en conflits avec %s %s definis a %s",
			      (this->space + "." + this-> token-> getStr ()).c_str (),
			      it-> type-> typeToString ().c_str (),
			      (this->space + "." + this-> token-> getStr ()).c_str (),
			      it-> token-> locusToString().c_str ());
		}
	    }
	    verifyPure ();	
	}	
    }
    
    void Function::verifyPure () {
	for (auto it : params) {
	    if (it->type != AstEnums::TYPEDVAR) return;
	}
	
	PureFrames::insertPure (PureFrame (this, space));	
    }
    
};
