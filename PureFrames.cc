#include "PureFrames.hh"
#include "UndefInfo.hh"
#include "VoidInfo.hh"
#include "Table.hh"

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


namespace Semantic {

    std::vector<PureFrame> PureFrames::frames;
    std::vector<FinalFrame> PureFrames::final_frames;

    void PureFrame::verifyReturn () {
    }

    void PureFrame::setMainParams (std::vector<Syntax::VarPtr> &) {	
    }
    
    void PureFrame::validate () {
	Syntax::Function * final = new Syntax::Function (fun -> token, space);
	Table::instance ().enterFrame (space);
	
	std::vector <Syntax::VarPtr> params;
	if (fun-> token-> getStr () == "main")
	    setMainParams (params);
	else  {
	    for (int i = 0; i < (int)fun->params.size (); i++) {
		params.push_back (fun->params[i]->declare (fun->token->getCstr ()));
	    }
	}
	    
	if (fun->type == Syntax::Var::empty ()) 
	    Table::instance().return_type () = new UndefInfo ();
	else {
	    auto type = fun-> type-> asType ();
	    if (type == NULL) return;
	    else 
		Table::instance().return_type () = type-> getType ()-> type;
	}
	    
	Syntax::BlockPtr block = (Syntax::BlockPtr)fun-> block-> instruction ();
	    
	auto info = Table::instance ().return_type ();
	Table::instance ().quitFrame ();
	if (info == NULL || info-> Is (UNDEF)) {
	    info = new VoidInfo;
	    //TODO verifier l'existance du type de retour
	} else verifyReturn ();
	    
	final-> info = info;
	final-> params = params;
	final-> block = block;
	
	PureFrames::insertFinal (FinalFrame (final, space));
    }

    void FinalFrame::finalize () {
    }
    
    

}
