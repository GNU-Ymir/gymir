#include <ymir/semantic/pack/UnPureFrame.hh>
#include <ymir/semantic/pack/Table.hh>
#include <ymir/semantic/types/_.hh>
#include <ymir/ast/TypedVar.hh>
#include <ymir/ast/Var.hh>
#include <ymir/syntax/Keys.hh>
#include <ymir/semantic/value/LambdaValue.hh>

namespace semantic {

    using namespace syntax;

    ILambdaFrame::ILambdaFrame (Namespace space, std::string & name, syntax::LambdaFunc frame) :
	IFrame (space, NULL)
    {
	this-> frame = frame;
	this-> name = name;
    }

    FrameProto ILambdaFrame::validate (const std::vector<InfoType> & params) {
	Table::instance ().enterFrame (this-> _space, this-> name, true);
	Table::instance ().enterBlock ();
	std::vector <Var> finalParams = IFrame::computeParams (this-> frame-> getParams (), params);

	if (this-> frame-> getExpr ()) {
	    return IFrame::validate (this-> name, this-> _space, finalParams, this-> frame-> getExpr ());
	} else {
	    return IFrame::validate (this-> name, this-> _space, finalParams, this-> frame-> getBlock ());
	}
    }

    ApplicationScore ILambdaFrame::isApplicable (ParamList params) {
	return IFrame::isApplicable (this-> frame-> token, this-> frame-> getParams (), params-> getParamTypes ());
    }
    
    FrameProto ILambdaFrame::validate (ParamList params) {
	return this-> validate (params-> getParamTypes ());
    }

    LambdaFunc ILambdaFrame::func () {
	return this-> frame;
    }
    
    std::string ILambdaFrame::getName () {
	return this-> name;
    }

    const char * ILambdaFrame::getId () {
	return ILambdaFrame::id ();
    }

    std::string ILambdaFrame::toString () {
	Ymir::OutBuffer buf ("#", this-> name, " (");
	for (auto it : Ymir::r (0, this-> frame-> getParams ().size ())) {
	    auto var = this-> frame-> getParams ()[it];
	    buf.write (var-> prettyPrint ());
	    if (it < (int) this-> frame-> getParams ().size () - 1)
		buf.write (", ");
	}
	buf.write (")");
	return buf.str ();
    }
    
    ILambdaValue::ILambdaValue (LambdaFrame frame)	
	: frame (frame)
    {}

    const char* ILambdaValue::getId () {
	return ILambdaValue::id ();
    }

    Value ILambdaValue::clone () {
	return this;
    }

    syntax::Expression ILambdaValue::toYmir (Symbol sym) {
	auto ret = new (Z0) ILambdaFunc (sym-> sym, this-> frame);
	ret-> info = sym;
	ret-> info-> value () = this;
	return ret;
    }

    std::string ILambdaValue::toString () {
	return Ymir::OutBuffer (this-> frame-> space ().toString (), ".",
				this-> frame-> getName ()).str ();
    }
    
    
}
