#include "Ast.hh"

namespace Syntax {

    Ast::Ast (Lexical::TokenPtr token, AstEnums::AstEnum type)
	: token (token),
	  type (type)
    {
    }
    
};

