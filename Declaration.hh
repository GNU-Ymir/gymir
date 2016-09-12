#pragma once

#include "Syntax.hh"

namespace Syntax {

    struct Declaration : Ast {
	Declaration (Lexical::TokenPtr ptr) : Ast(ptr) {}
    };
   
};
