#include <ymir/semantic/pack/_.hh>
#include "ast/Proto.hh"
#include "ast/ParamList.hh"

namespace semantic {
    using namespace syntax;
    
    std::vector <ExternFrame> IExternFrame::__extFrames__;
    
    IExternFrame::IExternFrame (Namespace space, std::string from, ::syntax::Proto func) :
	IFrame (space, NULL),
	_fr (NULL),
	_proto (func)
    {
	__extFrames__.push_back (this);
	this-> _name = func-> name ();
    }

    IExternFrame::IExternFrame (Namespace space, ::syntax::Function func) :
	IFrame (space, func),
	_fr (NULL),
	_proto (NULL)
    {
	__extFrames__.push_back (this);
	this-> _name = func-> name ();
    }

    ApplicationScore IExternFrame::isApplicable (ParamList params) {
	if (this-> _proto == NULL) return IFrame::isApplicable (params);
	if (this-> _proto-> isVariadic ()) return isApplicableVariadic (params);
	else return IFrame::isApplicable (this-> _proto-> ident, this-> _proto-> params (), params-> getParamTypes ());
    }

    ApplicationScore IExternFrame::isApplicableVariadic (ParamList params) {
	auto ftypes = params-> getParamTypes ();
	std::vector <InfoType> types;
	if (ftypes.size () >= this-> _proto-> params ().size ()) {
	    types = {ftypes.begin (), ftypes.begin () + this-> _proto-> params ().size ()};
	} else types = ftypes;
	auto ret = IFrame::isApplicable (this-> _proto-> ident, this-> _proto-> params (), types);
	
	if (ret != NULL) {
	    for (auto  it : Ymir::r (this-> _proto-> params ().size (), ftypes.size ())) {
		ret-> score += SAME;
		ret-> treat.push_back (NULL);
	    }
	}
	return ret;
    }
    
    FrameProto IExternFrame::validate () {
	//if (this-> _proto == NULL) return validateFunc ();
	auto ancSpace = Table::instance ().programNamespace ();
	Table::instance ().enterFrame (this-> space (), this-> name (), this-> isInternal ());
	std::vector <Var> finalParams = IFrame::computeParams (this-> _proto-> params ());
	Table::instance ().setCurrentSpace (Namespace (this-> space (), this-> name ()));
	Table::instance ().programNamespace () = this-> space ();

	if (this-> _proto-> type () == NULL) {
	    Table::instance ().retInfo ().info = new ISymbol (Word::eof (), new IVoidInfo ());
	} else {
	    Table::instance ().retInfo ().info = this-> _proto-> type ()-> asType ()-> info;
	}

	this-> _fr = new IFrameProto (this-> name (), this-> space (), Table::instance ().retInfo ().info, finalParams, this-> tempParams);

	this-> _fr-> externName () = this-> _from;
	this-> _fr-> isCVariadic () = this-> isVariadic ();
	Table::instance ().quitFrame ();
	Table::instance ().programNamespace () = ancSpace;
	return this-> _fr;
    }

    FrameProto IExternFrame::validate (::syntax::ParamList) {
	return validate ();
    }

    FrameProto IExternFrame::validate (std::vector <InfoType>) {
	return validate ();
    }

    Word IExternFrame::ident () {
	if (this-> _proto) return this-> _proto-> ident;
	return IFrame::ident ();
    }

    std::string IExternFrame::from () {
	return this-> _from;
    }

    FrameProto IExternFrame::proto () {
	return this-> _fr;
    }

    std::string IExternFrame::name () {
	return this-> _name;
    }

    bool IExternFrame::isVariadic () const {
    }

    FrameProto IExternFrame::validateFunc () {
    }

    const char* IExternFrame::getId () {
	return IExternFrame::id ();
    }
    
}
