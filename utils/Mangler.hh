#pragma once

#include <string>

namespace semantic {

    class IFrameProto;
    typedef IFrameProto* FrameProto;

    class IFinalFrame;
    typedef IFinalFrame* FinalFrame;
    
}

namespace Mangler {

    std::string mangle_file (std::string&);

    std::string mangle_function (std::string&, ::semantic::FrameProto);

    std::string mangle_function (std::string&, ::semantic::FinalFrame);
    
    std::string mangle_functionv (std::string&, ::semantic::FrameProto);

    std::string mangle_type (std::string);

    std::string mangle_namespace (std::string);

    std::string mangle_var (std::string);

    std::string mangle_global (std::string);
    
}
