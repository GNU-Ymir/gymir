#pragma once

#include <string>
#include <vector>

namespace syntax {
    class IExpression;
    typedef IExpression* Expression;
}

namespace semantic {

    class IFrameProto;
    typedef IFrameProto* FrameProto;

    class IFinalFrame;
    typedef IFinalFrame* FinalFrame;

    class IInfoType;
    typedef IInfoType* InfoType;
    
}

namespace Mangler {

    std::string mangle_file (std::string&);

    std::string mangle_function (std::string&, ::semantic::FrameProto);

    std::string mangle_function (std::string&, ::semantic::FinalFrame);
    
    std::string mangle_functionv (std::string&, ::semantic::FrameProto);

    std::string mangle_type (::semantic::InfoType, std::string);

    std::string mangle_namespace (std::string);

    std::string mangle_struct (std::string, char);

    std::string mangle_var (std::string);

    std::string mangle_global (std::string);

    std::string mangle_template_list (const std::vector <syntax::Expression> &);
    
}
