#pragma once

#include <gc/gc_cpp.h>
#include <ymir/ast/Var.hh>
#include <ymir/semantic/pack/Namespace.hh>

namespace semantic {

    class IInfoType;
    typedef IInfoType* InfoType;
    
    class IFrameProto : public gc {
    public:
	
	Namespace space;
	std::string name;
	InfoType type;
	std::vector <::syntax::Var> vars;

	
    };

    typedef IFrameProto* FrameProto;
    
}
