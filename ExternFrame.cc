#include <ymir/semantic/pack/_.hh>
#include "ast/Proto.hh"
#include "ast/ParamList.hh"
#include <ymir/syntax/Keys.hh>

namespace semantic {
    using namespace syntax;
    
    std::vector <ExternFrame> IExternFrame::__extFrames__;
    
    IExternFrame::IExternFrame (Namespace space, std::string from, ::syntax::Proto func) :
	IFrame (space, NULL),
	_proto (func),
	_from (from),
	_fr (NULL)	
    {
	__extFrames__.push_back (this);
	this-> _name = func-> name ();
    }

    IExternFrame::IExternFrame (Namespace space, ::syntax::Function func) :
	IFrame (space, func),
	_proto (NULL),
	_fr (NULL)
    {
	__extFrames__.push_back (this);
	this-> _name = func-> name ();
    }

    ApplicationScore IExternFrame::isApplicable (const std::vector<InfoType> & params) {
	if (this-> _proto == NULL) return IFrame::isApplicable (params);
	if (this-> _proto-> isVariadic ()) return isApplicableVariadic (params);
	else return IFrame::isApplicable (this-> _proto-> ident, this-> _proto-> params (), params);
    }
    
    ApplicationScore IExternFrame::isApplicable (ParamList params) {
	if (this-> _proto == NULL) return IFrame::isApplicable (params);
	if (this-> _proto-> isVariadic ()) return isApplicableVariadic (params-> getParamTypes ());
	else return IFrame::isApplicable (this-> _proto-> ident, this-> _proto-> params (), params-> getParamTypes ());
    }

    ApplicationScore IExternFrame::isApplicableVariadic (const std::vector <InfoType> & ftypes) {
	std::vector <InfoType> types;
	if (ftypes.size () >= this-> _proto-> params ().size ()) {
	    types = {ftypes.begin (), ftypes.begin () + this-> _proto-> params ().size ()};
	} else types = ftypes;
	auto ret = IFrame::isApplicable (this-> _proto-> ident, this-> _proto-> params (), types);
	
	if (ret != NULL) {
	    for (auto it __attribute__((unused)) : Ymir::r (this-> _proto-> params ().size (), ftypes.size ())) {
		ret-> score += SAME;
		ret-> treat.push_back (NULL);
	    }
	}
	return ret;
    }
        
    FrameProto IExternFrame::validate () {
	if (this-> _proto == NULL) return validateFunc ();
	auto ancSpace = Table::instance ().programNamespace ();
	Table::instance ().enterFrame (this-> space (), this-> name (), this-> templateParams (), this-> isInternal ());
	std::vector <Var> finalParams = IFrame::computeParams (this-> _proto-> params ());	
	Table::instance ().setCurrentSpace (Namespace (this-> space (), this-> name ()));
	Table::instance ().programNamespace () = this-> space ();

	if (this-> _proto-> type () == NULL) {
	    Table::instance ().retInfo ().info = new (Z0)  ISymbol (Word::eof (), new (Z0)  IVoidInfo ());
	} else {
	    auto type = this-> _proto-> type ()-> asType ();
	    if (type == NULL)
		Table::instance ().retInfo ().info = new (Z0) ISymbol (Word::eof (), new (Z0) IVoidInfo ());
	    else Table::instance ().retInfo ().info = type-> info;
	    Table::instance ().retInfo ().deco = this-> _proto-> type ()-> deco.getStr ();
	    if (Table::instance ().retInfo ().deco != Keys::REF && Table::instance ().retInfo ().deco != Keys::MUTABLE)
		Table::instance ().retInfo ().info-> isConst (true);
	}

	this-> _fr = new (Z0)  IFrameProto (this-> name (), this-> space (), Table::instance ().retInfo ().info, finalParams, this-> tempParams);

	this-> _fr-> externName () = this-> _from;
	this-> _fr-> isCVariadic () = this-> isVariadic ();
	Table::instance ().quitFrame ();
	Table::instance ().programNamespace () = ancSpace;
	return this-> _fr;
    }

    FrameProto IExternFrame::validate (::syntax::ParamList) {
	return validate ();
    }

    FrameProto IExternFrame::validate (const std::vector<InfoType> &) {
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
	return this-> _proto && this-> _proto-> isVariadic ();
    }

    FrameProto IExternFrame::validateFunc () {
	auto ancSpace = Table::instance ().programNamespace ();
	Table::instance ().enterFrame (this-> space (), this-> name (), this-> templateParams (), this-> isInternal ());
	std::vector <Var> finalParams = IFrame::computeParams (this-> _function-> getParams ());
	Table::instance ().setCurrentSpace (Namespace (this-> space (), this-> name ()));
	Table::instance ().programNamespace () = this-> space ();
	if (this-> _function-> getType () == NULL) {
	       Table::instance ().retInfo ().info = new (Z0)  ISymbol (Word::eof (), new (Z0)  IVoidInfo ());
	} else {
	    auto type = this-> _function-> getType ()-> asType ();
	    if (type) Table::instance ().retInfo ().info = type-> info;
	    else Table::instance ().retInfo ().info = new (Z0)  ISymbol (Word::eof (), new (Z0)  IVoidInfo ());
	}

	this-> _fr = new (Z0)  IFrameProto (this-> name (), this-> space (), Table::instance ().retInfo ().info, finalParams, this-> tempParams);
	this-> _fr-> externName () = this-> _from;
	this-> _fr-> isCVariadic () = this-> isVariadic ();
	Table::instance ().quitFrame ();
	Table::instance ().programNamespace () = ancSpace;
	return this-> _fr;	
    }

    const char* IExternFrame::getId () {
	return IExternFrame::id ();
    }

    
    std::string IExternFrame::toString () {
	if (this-> _proto) {
	    Ymir::OutBuffer buf;
	    buf.write ("(");
	    for (auto it : Ymir::r (0, this-> _proto-> params ().size ())) {
		auto var = this-> _proto-> params () [it];
		buf.write (var-> prettyPrint ());
		if (it < (int) this-> _proto-> params ().size () - 1)
		    buf.write (", ");
	    }
	
	    buf.write (")");
	
	    if (auto t = this-> _proto-> type ())
		buf.write ("-> ", t-> prettyPrint ());
	    return buf.str ();
	}
	return IFrame::toString ();
    }

    
}
