#include "Ast.hh"
#include "AstGC.hh"

namespace Syntax {

    Ast::Ast (Lexical::TokenPtr token, AstEnums::AstEnum type)
	: token (token),
	  type (type)
    {
	AstGC::instance().addInfo (this);
    }
};

