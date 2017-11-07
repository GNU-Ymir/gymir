#pragma once

#include "Function.hh"

#include <gc/gc_cpp.h>

namespace Semantic {

    struct Frame : gc {

	Frame (Syntax::Function * fun, const std::string & space)
	    : fun (fun),
	      space (space)
	{}

	std::string name () {
	    if (fun->token->getStr () == "main") return std::string ("main");
	    return this->space + "." + fun->token->getStr();
	}
	
    protected:
	
	Syntax::Function * fun;
	std::string space;
	
    };
    

}
