#include "Ast.hh"
#include "AstGC.hh"

namespace Syntax {

    Ast::Ast (Lexical::TokenPtr token, AstEnums::AstEnum type)
	: token (token),
	  type (type)
    {
	this->id = AstGC::instance().addInfo (this);
    }

    void Ast::free () {
	AstGC::instance ().free (id);
    }
    
};

