#include "PureFrames.hh"
#include "UndefInfo.hh"
#include "VoidInfo.hh"
#include "Table.hh"

namespace Semantic {

    std::vector<PureFrame> PureFrames::frames;
    std::vector<FinalFrame> PureFrames::final_frames;

    void PureFrame::verifyReturn () {
    }
    
    void PureFrame::validate () {
	Syntax::Function * final = new Syntax::Function (fun -> token, space);
	Table::instance ().enterFrame (space);

	std::vector <Syntax::VarPtr> params;
	for (int i = 0; i < (int)fun->params.size (); i++) {
	    params.push_back (fun->params[i]->declare (fun->token->getCstr()));
	}

	if (fun->type == Syntax::Var::empty ()) 
	    Table::instance().return_type () = new UndefInfo ();
	else
	    Table::instance().return_type () = fun->type->asType()->getType ();

	Syntax::BlockPtr block = (Syntax::BlockPtr)fun->block->instruction ();
		
	auto info = Table::instance().return_type ();
	Table::instance ().quitFrame ();
	if (info == NULL || info->Is (UNDEF)) {
	    info = new VoidInfo;
	    //TODO verifier l'existance du type de retour
	} else verifyReturn ();

	final->info = info;
	final->params = params;
	final->block = block;
	
	PureFrames::insertFinal (FinalFrame (final, space));
    }
    

}
