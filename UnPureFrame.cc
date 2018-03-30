#include <ymir/semantic/pack/UnPureFrame.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/semantic/types/_.hh>
#include <ymir/ast/TypedVar.hh>
#include <ymir/ast/Var.hh>
#include <ymir/syntax/Keys.hh>

namespace semantic {

    using namespace syntax;

    IUnPureFrame::IUnPureFrame (Namespace space, syntax::Function fun) :
	IFrame (space, fun)
    {
	if (fun)
	    this-> name = fun-> getIdent ().getStr ();
    }

    FrameProto IUnPureFrame::validate (const std::vector<InfoType> & params) {
	Table::instance ().enterFrame (this-> _space, this-> name, this-> templateParams (), this-> attributes (), this-> isInternal ());
	Table::instance ().enterBlock ();
	std::vector <Var> finalParams = IFrame::computeParams (this-> _function-> getParams (), params);
	Namespace from = Table::instance ().globalNamespace ();
	if (this-> _imutSpace != "")
	    from = Namespace (from, this-> _imutSpace);
	return IFrame::validate (this-> _space, from, finalParams, this-> isVariadic ());
    }

    std::vector <InfoType> IUnPureFrame::getParamTypes () {
	std::vector <InfoType> params;
	for (auto it : this-> _function-> getParams ()) {
	    if (auto tvar = it-> to <ITypedVar> ()) {
		params.push_back (tvar-> getType ());
	    } else return {};
	}
	return params;
    }
    
    FrameProto IUnPureFrame::validate (ParamList params) {
	return this-> validate (params-> getParamTypes ());
    }

    const char * IUnPureFrame::getId () {
	return IUnPureFrame::id ();
    }
    
}
