#include <ymir/semantic/pack/PureFrame.hh>

namespace semantic {
    
    virtual const char* IPureFrame::getId () {
	return IPureFrame::id ();
    }
}
