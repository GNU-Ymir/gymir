#include <ymir/semantic/pack/PureFrame.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/semantic/types/_.hh>
#include <ymir/ast/TypedVar.hh>
#include <ymir/ast/Var.hh>
#include <ymir/syntax/Keys.hh>

namespace semantic {

    using namespace syntax;
    
    IPureFrame::IPureFrame (Namespace space, syntax::Function fun) :
	IFrame (space, fun),
	proto (NULL)
    {
	if (fun) {
	    this-> name = fun-> getIdent ().getStr ();
	    this-> validate ();
	}
    }
    
    FrameProto IPureFrame::validate (syntax::ParamList) {
	return this-> validate ();
    }

    FrameProto IPureFrame::validate (const std::vector<InfoType> &) {
	return this-> validate ();
    }    
    
    FrameProto IPureFrame::validate () {
	if (this-> proto) return this-> proto;
	else if (this-> name == Keys::MAIN && !this-> pass)
	    return validateMain ();
	
	Table::instance ().enterFrame (this-> _space, this-> name, this-> isInternal ());
	Table::instance ().enterBlock ();
	std::vector <Var> finalParam = IFrame::computeParams (this-> _function-> getParams ());
	
	this-> proto = IFrame::validate (finalParam);
	delete this-> _function-> getBlock ();
	return this-> proto;
    }

    FrameProto IPureFrame::validateMain () {
	if (this-> _function-> getParams ().size () == 1) {
	    auto tok = this-> _function-> getParams () [0]-> token;
	    if (auto a = this-> _function-> getParams () [0]-> to<ITypedVar> ()) {
		auto type = a-> getType ();
		if (!type-> isSame (new (GC) IArrayInfo (true, new (GC) IStringInfo (true)))) {
		    Ymir::Error::assert ("TODO, erreur");
		} else {
		    auto str = Word (tok.getLocus (), "string");
		    this-> _function-> getParams () [0] = new (GC) ITypedVar (tok,
									 new (GC) IArrayVar (tok, new (GC) IVar (str))
		    );
		}
	    }
	} else if (this-> _function-> getParams ().size () != 0) {
	    Ymir::Error::assert ("TODO, erreur");
	}
	this-> pass = true;
	return validate ();
    }
    
    const char* IPureFrame::getId () {
	return IPureFrame::id ();
    }
}
