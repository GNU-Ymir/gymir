#include "semantic/pack/Frame.hh"
#include <ymir/semantic/types/InfoType.hh>
#include "ast/ParamList.hh"
#include "errors/Error.hh"

namespace semantic {

    IFrame::IFrame (Namespace space, ::syntax::Function func) :
	_space (space),
	_function (func)
    {}

    FrameProto IFrame::validate () {
	Ymir::Error::assert ("TODO");
	return NULL;
    }

    FrameProto IFrame::validate (::syntax::ParamList) {
	Ymir::Error::assert ("TODO");
	return NULL;
    }
    
    Namespace& IFrame::space () {
	return this-> _space;
    }

    int& IFrame::currentScore () {
	return this-> _currentScore;
    }

    bool& IFrame::isInternal () {
	return this-> _isInternal;
    }

    bool IFrame::isVariadic () const {
	return this-> _isVariadic;
    }

    void IFrame::isVariadic (bool isVariadic) {
	this-> _isVariadic = isVariadic;
    }

    void IFrame::verifyReturn (Word token, Symbol ret, FrameReturnInfo infos) {
    }

    ::syntax::Function IFrame::func () {
	return this-> _function;
    }

    Word IFrame::ident () {
	
    }
    

}
