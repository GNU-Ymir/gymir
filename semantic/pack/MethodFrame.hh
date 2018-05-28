#pragma once

#include "Frame.hh"
#include "FrameProto.hh"
#include <ymir/ast/TypeCreator.hh>
#include "UnPureFrame.hh"
#include "PureFrame.hh"
#include "TemplateFrame.hh"
#include <ymir/ast/ParamList.hh>

namespace semantic {

    class IUnPureMethodFrame : public IUnPureFrame {
    };

    class IPureMethodFrame : public IPureFrame {
    };

    class ITemplateMethodFrame : public ITemplateFrame {
    };
	
}
