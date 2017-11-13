#pragma once

#include <gc/gc_cpp.h>

namespace semantic {

    class IFinalFrame : public gc {
    };

    typedef IFinalFrame* FinalFrame;
    
}
