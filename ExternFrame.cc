#include "semantic/pack/ExternFrame.hh"
#include "ast/Proto.hh"
#include "ast/ParamList.hh"

namespace semantic {

    std::vector <ExternFrame> IExternFrame::__extFrames__;
    
    IExternFrame::IExternFrame (Namespace space, std::string from, ::syntax::Proto func) :
	IFrame (space, NULL),
	_fr (NULL),
	_proto (func)
    {
	__extFrames__.push_back (this);
    }

    IExternFrame::IExternFrame (Namespace space, ::syntax::Function func) :
	IFrame (space, func),
	_fr (NULL),
	_proto (NULL)
    {
	__extFrames__.push_back (this);
    }

    FrameProto IExternFrame::validate () {
    }

    FrameProto IExternFrame::validate (::syntax::ParamList) {
    }

    Word IExternFrame::ident () {
    }

    std::string IExternFrame::from () {
	return this-> _from;
    }

    FrameProto IExternFrame::proto () {
	return this-> _fr;
    }

    std::string IExternFrame::name () {
    }

    bool IExternFrame::isVariadic () const {
    }

    FrameProto IExternFrame::validateFunc () {
    }

}
