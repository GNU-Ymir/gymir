#pragma once
#include <string>

namespace semantic {

    class IFrameProto;
    typedef IFrameProto* FrameProto;
    
}

namespace Mangler {

    std::string mangle_file (std::string&);

    std::string mangle_function (std::string&, ::semantic::FrameProto);

    std::string mangle_functionv (std::string&, ::semantic::FrameProto);
    
}
