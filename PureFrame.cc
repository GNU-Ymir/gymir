#include <ymir/semantic/pack/PureFrame.hh>

namespace semantic {

    IPureFrame::IPureFrame (Namespace space, syntax::Function fun) :
	IFrame (space, fun)
    {
	if (fun) {
	    this-> name = fun-> getIdent ().getStr ();
	}
    }
    
    FrameProto IPureFrame::validate (syntax::ParamList) {
	return NULL;
    }

    FrameProto IPureFrame::validate () {
	return NULL;
    }

    FrameProto IPureFrame::validateMain () {
	return NULL;
    }
    
    const char* IPureFrame::getId () {
	return IPureFrame::id ();
    }
}
