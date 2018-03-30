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
	
	Table::instance ().enterFrame (this-> _space, this-> name, this-> templateParams (), this-> attributes (), this-> isInternal ());
	Table::instance ().enterBlock ();
	
	std::vector <Var> finalParam = IFrame::computeParams (this-> _function-> getParams ());
	this-> proto = IFrame::validate (finalParam);	
	return this-> proto;
    }

    FrameProto IPureFrame::validateMain () {
	FrameTable::instance ().addMain ();
	if (this-> _function-> getParams ().size () == 1) {
	    auto tok = this-> _function-> getParams () [0]-> token;
	    if (auto a = this-> _function-> getParams () [0]-> to<ITypedVar> ()) {
		auto type = a-> getType ();
		if (!type-> isSame (new (Z0)  IArrayInfo (true, new (Z0) IStringInfo (false)))) {
		    Ymir::Error::incompatibleTypes (a-> token, new (Z0) ISymbol (a-> token, type),  new (Z0)  IArrayInfo (true, new (Z0) IStringInfo (false)));
		}
	    } else {
		auto str = Word (tok.getLocus (), "string");
		this-> _function-> getParams () [0] = new (Z0)  ITypedVar (tok,
									   new (Z0)  IArrayVar (tok, new (Z0)  IVar (str))
		);
	    }	    
	} else if (this-> _function-> getParams ().size () != 0) {
	    Ymir::Error::mainPrototype (this-> _function-> getIdent ());
	    this-> _function-> getParams () = {};
	}
	this-> pass = true;
	return validate ();
    }

    bool IPureFrame::isPure () {
	return true;
    }

    std::vector <InfoType> IPureFrame::getParamTypes () {
	std::vector <InfoType> params;
	for (auto it : this-> _function-> getParams ()) {
	    params.push_back (it-> to<ITypedVar> ()-> getType ());
	}
	return params;
    }

    InfoType IPureFrame::getRetType () {
	auto proto = this-> validate ();
	return proto-> type ()-> type;
    }
    
    const char* IPureFrame::getId () {
	return IPureFrame::id ();
    }
}
