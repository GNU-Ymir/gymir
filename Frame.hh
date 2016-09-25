#pragma once

#include "Function.hh"

namespace Semantic {

    struct Frame {

	Frame (Syntax::Function * fun, const std::string & space)
	    : fun (fun),
	      space (space)
	{}

	std::string name () {
	    return this->space + "." + fun->token->getStr();
	}
	
    protected:
	
	Syntax::Function * fun;
	std::string space;
	
    };
    

}
